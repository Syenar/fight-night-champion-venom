"""Compile the attributed fnc_content source against a freshly built pinned SDK."""
import argparse
from hashlib import file_digest
import json, os
from pathlib import Path
from string import Template
import subprocess

PACKAGE=Path(__file__).resolve().parent

def digest(p):
    with p.open('rb') as f:return file_digest(f,'sha256').hexdigest()

def durable(p,data):
    if isinstance(data,str):data=data.encode('utf-8')
    p.parent.mkdir(parents=True,exist_ok=True)
    with p.open('wb') as f:f.write(data);f.flush();os.fsync(f.fileno())
    if p.read_bytes()!=data:raise ValueError(f'Write verification failed:{p}')

def main():
    a=argparse.ArgumentParser(description=__doc__)
    a.add_argument('--sdk-receipt',type=Path,required=True)
    a.add_argument('--clang23',type=Path,required=True)
    a.add_argument('--output',type=Path,required=True)
    args=a.parse_args()
    args.output=args.output.resolve();args.clang23=args.clang23.resolve()
    sdk=json.loads(args.sdk_receipt.read_text());profile_path=PACKAGE.parent/'title/title-build-profile.json';profile=json.loads(profile_path.read_text())
    if sdk['buildPassed']!=['runtime','gpu']:raise ValueError('Both source SDK stacks must have passed')
    version=subprocess.check_output([str(args.clang23),'--version'],text=True).splitlines()[0]
    if 'clang version 23.1.4 ' not in version:raise ValueError(version)
    args.output.mkdir(parents=True,exist_ok=False)
    o=args.output;temp=o/'temp';temp.mkdir();env=dict(os.environ,TEMP=str(temp),TMP=str(temp))
    root=Path(sdk['sdkSource']);build=Path(sdk['sdkBuild']);libs=Path(sdk['sdkLibs']);bundle=Path(sdk['runtimeBundle'])
    loc={'sdkSource':root.as_posix(),'sdkBuild':build.as_posix(),'sdkLibs':libs.as_posix(),'project':o.as_posix()}
    source=o/'src/fnc_content.cpp';durable(source,(PACKAGE/'fnc_content.cpp').read_bytes());obj=o/'fnc_content.obj';tool=o/'tools/fnc_content.exe'
    tool.parent.mkdir()
    quote=lambda p:'"'+p.as_posix()+'"'
    compile_rsp=o/'compile.rsp';durable(compile_rsp,Template(profile['compileProfiles']['mapping']).substitute(loc)+f' -o {quote(obj)} -c {quote(source)}\n')
    link_rsp=o/'link.rsp';durable(link_rsp,profile['linkFlags'].replace('/subsystem:windows','/subsystem:console')+' '+quote(obj)+' '+Template(profile['linkLibraries']).substitute(loc)+f' -o {quote(tool)}\n')
    commands=[]
    for name,rsp in (('compile',compile_rsp),('link',link_rsp)):
        argv=[str(args.clang23),'@'+rsp.as_posix()];commands.append(argv)
        with (o/f'{name}.log').open('wb') as f:
            r=subprocess.run(argv,cwd=o,env=env,stdout=f,stderr=subprocess.STDOUT);f.flush();os.fsync(f.fileno())
        if r.returncode:raise RuntimeError(f'{name}failed{r.returncode}:see{o/name}.log')
    for p in (obj,tool):
        with p.open('rb+') as f:f.flush();os.fsync(f.fileno())
    files={'fnc_content.exe':digest(tool)}
    for name in ('rexruntimerd.dll','TracyClientrd.dll'):
        if digest(bundle/name)!=sdk['dllSha256'][name]:raise ValueError(f'SDKDLLhashmismatch:{name}')
        durable(tool.parent/name,(bundle/name).read_bytes());files[name]=digest(tool.parent/name)
    receipt={'ToolRoot':str(tool.parent),'ToolFiles':files,'SourceSha256':digest(PACKAGE/'fnc_content.cpp'),'SdkBuildReceiptSha256':digest(args.sdk_receipt),'BuildProfileSha256':digest(profile_path),'Compiler':version,'Commands':commands,'ExecutedCompileResponseSha256':digest(compile_rsp),'ExecutedLinkResponseSha256':digest(link_rsp),'GameLaunched':False}
    durable(o/'content-tool-receipt.json',json.dumps(receipt,indent=2)+'\n')
    print(f'Source-builtcontenthelper:{tool}',flush=True)

if __name__=='__main__':main()