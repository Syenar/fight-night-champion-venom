#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <algorithm>
#include <emmintrin.h>
#include <filesystem>
#include <fstream>
#include <shellapi.h>
#include <shobjidl.h>
#include <shlobj.h>
#include <string>
#include <thread>
#include <vector>
namespace fs = std::filesystem;
constexpr UINT LogMessage = WM_APP + 1, BuildDoneMessage = WM_APP + 2;
constexpr DWORD PreviousOutputRequiredExit = 79;
constexpr int ReviewCommandId = 300, BuildCommandId = 301, LoadInputsId = 302,
              RememberInputsId = 303, ResumeBuildId = 304, OpenOutputId = 305, AdvancedId = 306, BuildToolsId = 307, InstallLocationBrowseId = 308;
enum FieldIndex
{
    SdkField,
    RetailField,
    UpdateField,
    DlcField,
    Clang23Field,
    Clang22Field,
    PythonField,
    OutputField,
    AptOverrideField,
    CmakeField,
    NinjaField,
    PowerShellField,
    JobsField,
    InstallField,
    PreviousOutputField
};
struct Field
{
    const wchar_t *key;
    const wchar_t *label;
    bool folder;
    bool hasBrowseButton;
    const wchar_t *initial;
};
const Field fields[] = {
    {L"sdk-checkout", L"Pinned SDK checkout (local Git objects)", true, true, L""},
    {L"game-data-root", L"Owned retail game folder (default.xex)", true, true, L""},
    {L"update-data-root", L"Exact TU2 folder (default.xexp)", true, true, L""},
    {L"dlc-title-root", L"Owned DLC title 45410915 folder", true, true, L""},
    {L"clang23", L"LLVM 23.1.4 clang++.exe", false, true, L""},
    {L"clang22", L"LLVM 22.1.8 clang++.exe", false, true, L""},
    {L"python", L"Python python.exe (backend checks features)", false, true, L""},
    {L"output", L"Private output (fresh folder; choose explicitly)", true, true, L""},
    {L"apt-tools-root", L"APT tools override (blank = bundled tools)", true, true, L""},
    {L"cmake", L"CMake executable or PATH command", false, true, L"cmake"},
    {L"ninja", L"Ninja executable or PATH command", false, true, L"ninja"},
    {L"powershell", L"PowerShell 7 executable or PATH command", false, true, L"pwsh"},
    {L"jobs", L"Parallel compiler jobs", false, false, L"2"},
    {L"install-root", L"Install folder override (blank = output/installed)", true, true, L""},
    {L"previous-output", L"Previous Venom output (optional)", true, true, L""}};
constexpr int FieldCount = sizeof(fields) / sizeof(fields[0]);
HWND mainWindow, fieldInputs[FieldCount], fieldLabels[FieldCount], browseButtons[FieldCount],
    introductionLabel, progressLog, statusLabel, reviewButton, buildButton, loadButton, rememberBox,
    resumeBox, openButton, advancedButton, buildToolsButton,
    installLocationLabel, installLocationInput, installLocationBrowse;
HFONT uiFont;
bool running = false, simpleMode = true, simpleStarted = false, needsBuildTools = false;
bool needsPreviousOutput = true;
fs::path packageRoot, activeOutput, localInstallPython, localInstallSource, localInstallLocation;
std::wstring windowText(HWND controlHandle)
{
    int textLength = GetWindowTextLengthW(controlHandle);
    std::wstring result(textLength + 1, L'\0');
    GetWindowTextW(controlHandle, result.data(), textLength + 1);
    result.resize(textLength);
    return result;
}
std::wstring quoteArgument(const std::wstring &argument)
{
    // MS CRT argument quoting, including trailing backslashes. No shell is used.
    std::wstring quoted = L"\"";
    size_t backslashCount = 0;
    for (wchar_t character : argument)
    {
        if (character == L'\\')
        {
            ++backslashCount;
            continue;
        }
        quoted.append(character == L'\"' ? backslashCount * 2 + 1 : backslashCount, L'\\');
        backslashCount = 0;
        quoted += character;
    }
    quoted.append(backslashCount * 2, L'\\');
    return quoted + L"\"";
}
std::wstring makeCommandLine(const std::vector<std::wstring> &arguments)
{
    std::wstring commandLine;
    for (const auto &argument : arguments)
    {
        if (!commandLine.empty())
        {
            commandLine += L' ';
        }
        commandLine += quoteArgument(argument);
    }
    return commandLine;
}
std::wstring decodeUtf8(const std::string &bytes)
{
    if (bytes.empty())
    {
        return {};
    }
    int characterCount =
        MultiByteToWideChar(CP_UTF8, 0, bytes.data(), (int)bytes.size(), nullptr, 0);
    std::wstring result(characterCount, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, bytes.data(), (int)bytes.size(), result.data(), characterCount);
    return result;
}
void postLog(const std::wstring &text)
{
    auto message = new std::wstring(text);
    if (!PostMessageW(mainWindow, LogMessage, 0, (LPARAM)message))
    {
        delete message;
    }
}
void appendLog(const std::wstring &message)
{
    if (GetWindowTextLengthW(progressLog) > 220000)
    {
        SendMessageW(progressLog, EM_SETSEL, 0, 60000);
        SendMessageW(progressLog, EM_REPLACESEL, FALSE,
                     (LPARAM)L"[Older display text trimmed; full logs remain in output.]\r\n");
    }
    SendMessageW(progressLog, EM_SETSEL, -1, -1);
    SendMessageW(progressLog, EM_REPLACESEL, FALSE, (LPARAM)message.c_str());
    SendMessageW(progressLog, EM_SCROLLCARET, 0, 0);
}
HRESULT configureIsoPicker(IFileDialog *dialog, const fs::path &initial)
{
    const COMDLG_FILTERSPEC filters[] = {{L"Xbox disc images (*.iso)", L"*.iso"}};
    auto result = dialog->SetFileTypes(1, filters);
    if (SUCCEEDED(result))
        result = dialog->SetFileTypeIndex(1);
    if (SUCCEEDED(result) && initial.is_absolute())
    {
        IShellItem *folder = nullptr;
        if (SUCCEEDED(SHCreateItemFromParsingName(initial.c_str(), nullptr, IID_PPV_ARGS(&folder))))
        {
            dialog->SetFolder(folder);
            folder->Release();
        }
    }
    return result;
}

std::wstring selectPath(bool folder, const wchar_t *title, bool iso = false, const fs::path &initial = {})
{
    IFileDialog *dialog = nullptr;
    if (FAILED(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER,
                                IID_PPV_ARGS(&dialog))))
    {
        return {};
    }
    DWORD options = 0;
    dialog->GetOptions(&options);
    dialog->SetOptions(options | FOS_FORCEFILESYSTEM | FOS_NOCHANGEDIR |
                       (folder ? FOS_PICKFOLDERS : FOS_FILEMUSTEXIST));
    dialog->SetTitle(title);
    if (iso && FAILED(configureIsoPicker(dialog, initial)))
    {
        dialog->Release();
        return {};
    }
    std::wstring selectedPath;
    if (SUCCEEDED(dialog->Show(mainWindow)))
    {
        IShellItem *item = nullptr;
        if (SUCCEEDED(dialog->GetResult(&item)))
        {
            PWSTR fileSystemPath = nullptr;
            if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &fileSystemPath)))
            {
                selectedPath = fileSystemPath;
                CoTaskMemFree(fileSystemPath);
            }
            item->Release();
        }
    }
    dialog->Release();
    return selectedPath;
}
bool hasReparsePoint(fs::path candidate)
{
    for (; !candidate.empty();)
    {
        DWORD attributes = GetFileAttributesW(candidate.c_str());
        if (attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_REPARSE_POINT))
        {
            return true;
        }
        auto parent = candidate.parent_path();
        if (parent == candidate)
        {
            break;
        }
        candidate = parent;
    }
    return false;
}
bool isSameOrInside(const fs::path &candidate, const fs::path &parent)
{
    auto candidatePart = candidate.begin();
    for (auto parentPart = parent.begin(); parentPart != parent.end();
         ++candidatePart, ++parentPart)
    {
        if (candidatePart == candidate.end() ||
            _wcsicmp(candidatePart->c_str(), parentPart->c_str()))
        {
            return false;
        }
    }
    return true;
}
std::vector<std::wstring> collectInputs()
{
    std::vector<std::wstring> inputValues;
    for (auto inputHandle : fieldInputs)
    {
        inputValues.push_back(windowText(inputHandle));
    }
    return inputValues;
}
std::vector<std::wstring> makePipelineArguments(const std::vector<std::wstring> &inputValues,
                                                bool resume)
{
    std::vector<std::wstring> arguments = {
        inputValues[PythonField], L"-u", L"-B",
        (packageRoot / L"native/installer/Build-Install-Venom.py").wstring()};
    for (int fieldIndex = 0; fieldIndex < FieldCount; ++fieldIndex)
    {
        if (fieldIndex == PythonField || inputValues[fieldIndex].empty())
        {
            continue;
        }
        arguments.push_back(L"--" + std::wstring(fields[fieldIndex].key));
        arguments.push_back(inputValues[fieldIndex]);
    }
    if (resume)
    {
        arguments.push_back(L"--resume");
    }
    return arguments;
}
bool validateInputs(const std::vector<std::wstring> &inputValues, bool resume, std::wstring &error)
{
    for (int fieldIndex = 0; fieldIndex < FieldCount; ++fieldIndex)
    {
        if (inputValues[fieldIndex].empty() && fieldIndex != AptOverrideField &&
            fieldIndex != InstallField && fieldIndex != PreviousOutputField)
        {
            error = L"Select or enter: " + std::wstring(fields[fieldIndex].label);
            return false;
        }
        if (inputValues[fieldIndex].find_first_of(L"\r\n") != std::wstring::npos)
        {
            error = L"Inputs cannot contain newlines.";
            return false;
        }
    }
    if (inputValues[JobsField].size() > 4 ||
        inputValues[JobsField].find_first_not_of(L"0123456789") != std::wstring::npos ||
        std::stoul(inputValues[JobsField]) == 0)
    {
        error = L"Jobs must be a positive integer (1 to 9999).";
        return false;
    }
    for (int fieldIndex : {SdkField, RetailField, UpdateField, DlcField, Clang23Field, Clang22Field,
                           PythonField, AptOverrideField, PreviousOutputField})
    {
        if (inputValues[fieldIndex].empty())
        {
            continue;
        }
        fs::path inputPath(inputValues[fieldIndex]);
        if (!inputPath.is_absolute() ||
            (fields[fieldIndex].folder ? !fs::is_directory(inputPath)
                                       : !fs::is_regular_file(inputPath)))
        {
            error = L"Select an existing absolute path: " + std::wstring(fields[fieldIndex].label);
            return false;
        }
    }
    fs::path outputPath(inputValues[OutputField]);
    if (!outputPath.is_absolute() || outputPath == outputPath.root_path())
    {
        error = L"Select an absolute private output below a drive root.";
        return false;
    }
    outputPath = outputPath.lexically_normal();
    if (hasReparsePoint(outputPath))
    {
        error = L"Private output must not use a junction or symbolic link.";
        return false;
    }
    if (!inputValues[InstallField].empty())
    {
        fs::path installPath(inputValues[InstallField]);
        if (!installPath.is_absolute() ||
            !isSameOrInside(installPath.lexically_normal(), outputPath) ||
            installPath.lexically_normal() == outputPath || hasReparsePoint(installPath))
        {
            error = L"Install override must be an absolute child folder within private output.";
            return false;
        }
    }
    for (int fieldIndex : {SdkField, RetailField, UpdateField, DlcField, AptOverrideField})
    {
        if (inputValues[fieldIndex].empty())
        {
            continue;
        }
        auto inputPath = fs::weakly_canonical(inputValues[fieldIndex]);
        if (isSameOrInside(outputPath, inputPath) || isSameOrInside(inputPath, outputPath))
        {
            error = L"Private output must be outside every read-only input.";
            return false;
        }
    }
    auto sourceRoot = fs::weakly_canonical(packageRoot);
    if (isSameOrInside(outputPath, sourceRoot) || isSameOrInside(sourceRoot, outputPath))
    {
        error = L"Private output must be outside the extracted source package.";
        return false;
    }
    if (fs::weakly_canonical(inputValues[RetailField]) ==
        fs::weakly_canonical(inputValues[UpdateField]))
    {
        error = L"Retail and TU2 folders must be distinct.";
        return false;
    }
    if (fs::exists(outputPath) && !resume)
    {
        error = L"Output already exists. Choose a fresh folder, or explicitly enable Resume for an "
                L"earlier pipeline run.";
        return false;
    }
    if (resume && !fs::is_regular_file(outputPath / L"pipeline-inputs.json"))
    {
        error = L"Resume requires the original pipeline-inputs.json in this output.";
        return false;
    }
    if (!fs::is_regular_file(packageRoot / L"native/installer/Build-Install-Venom.py"))
    {
        error = L"Source payload missing. Keep VenomInstaller.exe beside the native folder.";
        return false;
    }
    return true;
}
#include "native_ui_self_test.h"
std::thread ownedBuildThread;

void saveInputs(const fs::path &outputPath, const std::vector<std::wstring> &inputValues)
{
    if (!fs::is_regular_file(outputPath / L"pipeline-inputs.json") || hasReparsePoint(outputPath))
    {
        return;
    }
    auto settingsPath = outputPath / L"installer-ui-inputs.ini";
    if (hasReparsePoint(settingsPath))
    {
        return;
    }
    // A UTF-16 BOM makes Windows preserve international path names in the INI file.
    HANDLE file = CreateFileW(settingsPath.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_NEW,
                              FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file != INVALID_HANDLE_VALUE)
    {
        const unsigned char bom[] = {0xff, 0xfe};
        DWORD bytesWritten = 0;
        WriteFile(file, bom, 2, &bytesWritten, nullptr);
        CloseHandle(file);
    }
    for (int fieldIndex = 0; fieldIndex < FieldCount; ++fieldIndex)
    {
        if (!WritePrivateProfileStringW(L"Inputs", fields[fieldIndex].key,
                                        inputValues[fieldIndex].c_str(), settingsPath.c_str()))
        {
            postLog(L"[Could not remember an input inside selected output.]\r\n");
        }
    }
}
void loadInputs()
{
    auto directory =
        selectPath(true, L"Choose previous private output with installer-ui-inputs.ini");
    if (directory.empty())
    {
        return;
    }
    auto settingsPath = fs::path(directory) / L"installer-ui-inputs.ini";
    if (!fs::is_regular_file(settingsPath))
    {
        MessageBoxW(mainWindow, L"No remembered installer inputs in this folder.", L"Load inputs",
                    MB_OK);
        return;
    }
    for (int fieldIndex = 0; fieldIndex < FieldCount; ++fieldIndex)
    {
        wchar_t value[32768];
        GetPrivateProfileStringW(L"Inputs", fields[fieldIndex].key, fields[fieldIndex].initial,
                                 value, 32768, settingsPath.c_str());
        SetWindowTextW(fieldInputs[fieldIndex], value);
    }
    SetWindowTextW(fieldInputs[OutputField], directory.c_str());
    SendMessageW(resumeBox, BM_SETCHECK, BST_UNCHECKED, 0);
    SetWindowTextW(statusLabel,
                   L"Inputs loaded. Review paths; explicitly select Resume to reuse this output.");
}
struct Tail
{
    fs::path logPath;
    unsigned long long offset = 0;
};
void updateLogTails(const fs::path &outputPath, std::vector<Tail> &tails)
{
    // Bounded exact log directories; do not walk owned media or output recursively.
    for (auto directory : {outputPath / L"logs", outputPath / L"sdk/logs",
                           outputPath / L"title/logs", outputPath / L"content-tool/logs"})
    {
        std::error_code filesystemError;
        if (!fs::is_directory(directory, filesystemError))
        {
            continue;
        }
        for (fs::directory_iterator entry(directory, filesystemError), endIterator;
             entry != endIterator && !filesystemError; entry.increment(filesystemError))
        {
            if (!entry->is_regular_file(filesystemError) || entry->path().extension() != L".log")
            {
                continue;
            }
            auto logPath = entry->path();
            if (std::none_of(tails.begin(), tails.end(),
                             [&](const Tail &logTail) { return logTail.logPath == logPath; }))
            {
                tails.push_back({logPath, 0});
            }
        }
    }
    for (auto &logTail : tails)
    {
        std::error_code filesystemError;
        auto size = fs::file_size(logTail.logPath, filesystemError);
        if (filesystemError || size <= logTail.offset)
        {
            continue;
        }
        std::ifstream stream(logTail.logPath, std::ios::binary);
        if (!stream)
        {
            continue;
        }
        stream.seekg(logTail.offset);
        char buffer[16384];
        stream.read(buffer, sizeof(buffer));
        auto bytesRead = stream.gcount();
        if (bytesRead > 0)
        {
            logTail.offset += (unsigned long long)bytesRead;
            postLog(L"\r\n[" + logTail.logPath.filename().wstring() + L"]\r\n" +
                    decodeUtf8(std::string(buffer, (size_t)bytesRead)));
        }
    }
}
HANDLE createOwnedBuildJob()
{
    HANDLE job = CreateJobObjectW(nullptr, nullptr);
    if (!job)
    {
        throw std::runtime_error("Cannot create installer-owned build job");
    }
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits{};
    limits.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
    if (!SetInformationJobObject(job, JobObjectExtendedLimitInformation, &limits, sizeof(limits)))
    {
        CloseHandle(job);
        throw std::runtime_error("Cannot contain installer-owned build processes");
    }
    return job;
}

fs::path systemPowerShell()
{
    wchar_t windows[32768]{};
    const UINT size = GetSystemWindowsDirectoryW(windows, 32768);
    if (!size || size >= 32768) return {};
    return fs::path(windows) / L"System32/WindowsPowerShell/v1.0/powershell.exe";
}

std::wstring powerShellLiteral(const std::wstring &value)
{
    std::wstring out = L"'";
    for (wchar_t character : value)
    {
        out += character;
        if (character == L'\'') out += character;
    }
    return out + L"'";
}

std::vector<std::wstring> makeQuickArguments(const std::vector<std::wstring> &values)
{
    const auto installer = packageRoot / L"native/installer";
    const auto systemShell = systemPowerShell();
    if (!systemShell.empty() && fs::path(values[PythonField]) == systemShell)
    {
        std::ifstream script(installer / L"Quick-Python.ps1", std::ios::binary);
        if (!script) throw std::runtime_error("The portable Python setup script is missing.");
        const std::string body{std::istreambuf_iterator<char>(script), {}};
        if (body.empty() || body.size() > 16000)
            throw std::runtime_error("The portable Python setup script is incomplete.");
        auto command = L"& {\r\n" + decodeUtf8(body) + L"\r\n} -InstallerDirectory " +
                       powerShellLiteral(installer.wstring()) + L" -GameFolder " +
                       powerShellLiteral(values[RetailField]) + L" -Output " +
                       powerShellLiteral(values[OutputField]);
        if (!values[PreviousOutputField].empty())
            command += L" -PreviousOutput " + powerShellLiteral(values[PreviousOutputField]);
        return {values[PythonField], L"-NoProfile", L"-NonInteractive", L"-Command", command};
    }
    std::vector<std::wstring> args = {values[PythonField], L"-u", L"-B",
        (installer / L"Local-Copy-Setup.py").wstring(),
        L"--game-folder", values[RetailField], L"--output", values[OutputField]};
    if (!localInstallPython.empty())
    {
        args.insert(args.begin() + 3, {L"-c",
            L"import runpy,sys; from pathlib import Path; script=sys.argv.pop(1); "
            L"sys.argv[0]=script; sys.path.insert(0,str(Path(script).parent)); "
            L"runpy.run_path(script,run_name='__main__')"});
        // The local-copy entry point only installs existing files.
    }
    if (!values[PreviousOutputField].empty())
    {
        args.emplace_back(L"--previous-output");
        args.emplace_back(values[PreviousOutputField]);
    }
    return args;
}

void runPipeline(std::vector<std::wstring> inputValues, bool resume, bool remember, bool quick)
{
    lastChildPid.store(0);
    lastChildCode.store(STILL_ACTIVE);
    lastChildExited.store(false);
    lastOwnedJobDrained.store(false);
    lastMonitorFailed.store(false);
    lastJobAssigned.store(false);
    DWORD resultCode = ERROR_GEN_FAILURE;
    DWORD childExitCode = STILL_ACTIVE;
    HANDLE buildJob = nullptr;
    bool childAssignedToJob = false;
    HANDLE readPipe = nullptr, writePipe = nullptr, nullInput = INVALID_HANDLE_VALUE;
    PROCESS_INFORMATION process{};
    try
    {
        buildJob = createOwnedBuildJob();
        SECURITY_ATTRIBUTES security{sizeof(security), nullptr, TRUE};
        if (!CreatePipe(&readPipe, &writePipe, &security, 0) ||
            !SetHandleInformation(readPipe, HANDLE_FLAG_INHERIT, 0))
        {
            throw std::runtime_error("Cannot create progress pipe");
        }
        STARTUPINFOEXW startup{};
        startup.StartupInfo.cb = sizeof(startup);
        startup.StartupInfo.dwFlags = STARTF_USESTDHANDLES;
        startup.StartupInfo.hStdOutput = writePipe;
        startup.StartupInfo.hStdError = writePipe;
        nullInput = CreateFileW(L"NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, &security,
                                OPEN_EXISTING, 0, nullptr);
        if (nullInput == INVALID_HANDLE_VALUE)
        {
            throw std::runtime_error("Cannot open a noninteractive process input");
        }
        startup.StartupInfo.hStdInput = nullInput;
        SIZE_T attributeBytes = 0;
        InitializeProcThreadAttributeList(nullptr, 1, 0, &attributeBytes);
        std::vector<unsigned char> attributes(attributeBytes);
        startup.lpAttributeList = (LPPROC_THREAD_ATTRIBUTE_LIST)attributes.data();
        if (!InitializeProcThreadAttributeList(startup.lpAttributeList, 1, 0, &attributeBytes))
        {
            throw std::runtime_error("Cannot initialize process attributes");
        }
        HANDLE handles[] = {writePipe, nullInput};
        if (!UpdateProcThreadAttribute(startup.lpAttributeList, 0,
                                       PROC_THREAD_ATTRIBUTE_HANDLE_LIST, handles, sizeof(handles),
                                       nullptr, nullptr))
        {
            DeleteProcThreadAttributeList(startup.lpAttributeList);
            throw std::runtime_error("Cannot restrict inherited handles");
        }
        auto arguments = quick ? makeQuickArguments(inputValues)
                               : makePipelineArguments(inputValues, resume);
        auto commandLine = makeCommandLine(arguments);
        auto environment = selfTestChildEnvironment();
        BOOL created = CreateProcessW(
            inputValues[PythonField].c_str(), commandLine.data(), nullptr, nullptr, TRUE,
            CREATE_NO_WINDOW | CREATE_SUSPENDED | EXTENDED_STARTUPINFO_PRESENT |
                (uiSelfTest ? CREATE_UNICODE_ENVIRONMENT : 0),
            environment.empty() ? nullptr : environment.data(), packageRoot.c_str(),
            &startup.StartupInfo, &process);
        DWORD createError = GetLastError();
        DeleteProcThreadAttributeList(startup.lpAttributeList);
        CloseHandle(nullInput);
        nullInput = INVALID_HANDLE_VALUE;
        CloseHandle(writePipe);
        writePipe = nullptr;
        if (!created)
        {
            resultCode = createError;
            throw std::runtime_error("Cannot start selected Python executable");
        }
        lastChildPid.store(process.dwProcessId);
        if (!AssignProcessToJobObject(buildJob, process.hProcess))
        {
            throw std::runtime_error("Cannot assign suspended Python to installer-owned job");
        }
        childAssignedToJob = true;
        lastJobAssigned.store(true);
        if (ResumeThread(process.hThread) == static_cast<DWORD>(-1))
        {
            throw std::runtime_error("Cannot resume installer-owned Python process");
        }
        CloseHandle(process.hThread);
        process.hThread = nullptr;
        fs::path outputPath(inputValues[OutputField]);
        std::vector<Tail> tails;
        bool saved = false;
        unsigned tick = 0;
        for (;;)
        {
            if (selfTestCancelRequested.load())
            {
                throw std::runtime_error("Native UI self-test cancelled its owned build");
            }
            DWORD available = 0;
            if (PeekNamedPipe(readPipe, nullptr, 0, nullptr, &available, nullptr) && available)
            {
                char buffer[8192];
                DWORD bytesRead = 0;
                if (ReadFile(readPipe, buffer, std::min<DWORD>(available, sizeof(buffer)),
                             &bytesRead, nullptr) &&
                    bytesRead)
                {
                    postLog(decodeUtf8(std::string(buffer, bytesRead)));
                }
            }
            if (remember && !saved && fs::is_regular_file(outputPath / L"pipeline-inputs.json"))
            {
                saveInputs(outputPath, inputValues);
                saved = true;
                postLog(L"\r\n[Inputs remembered inside selected private output.]\r\n");
            }
            if (++tick % 10 == 0)
            {
                updateLogTails(outputPath, tails);
            }
            DWORD waitStatus = WaitForSingleObject(process.hProcess, 100);
            if (waitStatus == WAIT_FAILED)
            {
                throw std::runtime_error("Cannot monitor installer-owned Python process");
            }
            if (waitStatus == WAIT_OBJECT_0)
            {
                if (!GetExitCodeProcess(process.hProcess, &childExitCode))
                {
                    throw std::runtime_error("Cannot read installer-owned Python exit code");
                }
                resultCode = childExitCode;
                break;
            }
        }
        for (;;)
        {
            DWORD available = 0;
            if (!PeekNamedPipe(readPipe, nullptr, 0, nullptr, &available, nullptr) || !available)
            {
                break;
            }
            char buffer[8192];
            DWORD bytesRead = 0;
            if (!ReadFile(readPipe, buffer, std::min<DWORD>(available, sizeof(buffer)), &bytesRead,
                          nullptr) ||
                !bytesRead)
            {
                break;
            }
            postLog(decodeUtf8(std::string(buffer, bytesRead)));
        }
        updateLogTails(outputPath, tails);
        postLog(L"\r\n[Pipeline process exit code: " + std::to_wstring(resultCode) + L"]\r\n");
        if (resultCode == 0)
        {
            if (!fs::is_regular_file(outputPath / L"build-install-receipt.json"))
            {
                resultCode = ERROR_INVALID_DATA;
                postLog(L"[Exit 0 lacked required receipt; completion unverified.]\r\n");
            }
            else
            {
                postLog(L"[Build/install finished; review build-install-receipt.json. Runtime, DLC "
                        L"playability and release acceptance require separate verification. No "
                        L"game launched.]\r\n");
            }
        }
        else if (resultCode != 0 && (!quick || resultCode != PreviousOutputRequiredExit))
        {
            postLog(L"[Build/install failed. Preserve output and inspect logs. Incomplete stages "
                    L"are not overwritten.]\r\n");
        }
    }
    catch (const std::exception &error)
    {
        lastMonitorFailed.store(true);
        resultCode = ERROR_GEN_FAILURE;
        postLog(L"\r\n[Installer error: " + decodeUtf8(error.what()) + L"; Windows error " +
                std::to_wstring(GetLastError()) + L"]\r\n");
        if (process.hProcess && WaitForSingleObject(process.hProcess, 0) == WAIT_TIMEOUT)
        {
            // Only this installer's owned process tree is stopped. Unrelated games and builds
            // are never located or controlled by name.
            if (childAssignedToJob)
            {
                TerminateJobObject(buildJob, ERROR_GEN_FAILURE);
                CloseHandle(buildJob);
                buildJob = nullptr;
            }
            else
            {
                // The child is still suspended and has never executed or spawned descendants.
                TerminateProcess(process.hProcess, ERROR_GEN_FAILURE);
            }
            WaitForSingleObject(process.hProcess, uiSelfTest ? 5000 : INFINITE);
            GetExitCodeProcess(process.hProcess, &childExitCode);
            postLog(L"[Installer-owned build stopped after monitoring failure; incomplete output "
                    L"is preserved.]\r\n");
        }
    }
    if (buildJob)
    {
        if (uiSelfTest)
        {
            lastOwnedJobDrained.store(drainSelfTestJob(buildJob));
            if (!lastOwnedJobDrained.load())
            {
                resultCode = ERROR_TIMEOUT;
            }
        }
        CloseHandle(buildJob);
    }
    lastChildCode.store(childExitCode);
    if (process.hProcess)
    {
        lastChildExited.store(WaitForSingleObject(process.hProcess, 0) == WAIT_OBJECT_0);
    }
    if (process.hProcess)
    {
        CloseHandle(process.hProcess);
    }
    if (process.hThread)
    {
        CloseHandle(process.hThread);
    }
    if (readPipe)
    {
        CloseHandle(readPipe);
    }
    if (writePipe)
    {
        CloseHandle(writePipe);
    }
    if (nullInput != INVALID_HANDLE_VALUE)
    {
        CloseHandle(nullInput);
    }
    PostMessageW(mainWindow, BuildDoneMessage, resultCode, 0);
}
void setInputsEnabled(bool enabled)
{
    for (int fieldIndex = 0; fieldIndex < FieldCount; ++fieldIndex)
    {
        EnableWindow(fieldInputs[fieldIndex], enabled);
        if (browseButtons[fieldIndex])
        {
            EnableWindow(browseButtons[fieldIndex], enabled);
        }
    }
    for (HWND controlHandle : {reviewButton, buildButton, loadButton, rememberBox, resumeBox, advancedButton, installLocationInput, installLocationBrowse})
    {
        EnableWindow(controlHandle, enabled);
    }
}
void reviewAndStart(bool build)
{
    if (running)
    {
        return;
    }
    try
    {
        auto inputValues = collectInputs();
        bool resume = SendMessageW(resumeBox, BM_GETCHECK, 0, 0) == BST_CHECKED;
        std::wstring error;
        if (!validateInputs(inputValues, resume, error))
        {
            if (uiSelfTest)
            {
                throw std::runtime_error(strictUtf8(error));
            }
            MessageBoxW(mainWindow, error.c_str(), L"Review inputs", MB_OK | MB_ICONWARNING);
            return;
        }
        auto message =
            L"Read-only inputs: retail game, exact TU2, owned DLC, SDK and tools.\r\nPrivate "
            L"output: " +
            inputValues[OutputField] +
            L"\r\n\r\nThe backend verifies exact hashes and locked sources, builds locally and "
            L"installs without launching. Logs and receipts remain in output.\r\n\r\nExact command "
            L"(direct Python invocation, no shell):\r\n" +
            makeCommandLine(makePipelineArguments(inputValues, resume)) + L"\r\n\r\n" +
            (build ? L"Start this build/install now?" : L"Use Build and install when ready.");
        bool confirmed =
            uiSelfTest ? selfTestConfirmation(build, inputValues, resume)
                       : MessageBoxW(mainWindow, message.c_str(),
                                     build ? L"Confirm build and install" : L"Review exact command",
                                     MB_ICONINFORMATION | (build ? MB_OKCANCEL : MB_OK)) == IDOK;
        if (!confirmed || !build)
        {
            return;
        }
        activeOutput = inputValues[OutputField];
        running = true;
        setInputsEnabled(false);
        SetWindowTextW(statusLabel, L"Build/install running. Stage output and compiler logs appear "
                                    L"below. The game will not launch.");
        appendLog(L"\r\n[Starting selected Python and pinned build/install pipeline.]\r\n");
        ownedBuildThread = std::thread(runPipeline, inputValues, resume,
                                       SendMessageW(rememberBox, BM_GETCHECK, 0, 0) == BST_CHECKED, false);
    }
    catch (const std::exception &error)
    {
        running = false;
        setInputsEnabled(true);
        if (uiSelfTest)
        {
            uiSelfTest->failure = error.what();
        }
        else
        {
            MessageBoxW(mainWindow, decodeUtf8(error.what()).c_str(), L"Input error",
                        MB_OK | MB_ICONERROR);
        }
    }
}
fs::path findInstalledPython()
{
    wchar_t buffer[32768];
    const DWORD count = SearchPathW(nullptr, L"python.exe", nullptr, 32768, buffer, nullptr);
    if (count && count < 32768)
    {
        fs::path path(buffer);
        auto text = path.wstring();
        std::transform(text.begin(), text.end(), text.begin(), [](wchar_t ch) { return towlower(ch); });
        if (text.find(L"windowsapps") == std::wstring::npos && fs::is_regular_file(path))
            return path;
    }
    const DWORD size = GetEnvironmentVariableW(L"LOCALAPPDATA", buffer, 32768);
    if (!size || size >= 32768) return {};
    const auto base = fs::path(buffer) / L"Programs/Python";
    std::error_code error;
    for (const auto &child : fs::directory_iterator(base, error))
    {
        const auto exe = child.path() / L"python.exe";
        if (fs::is_regular_file(exe, error)) return exe;
    }
    return {};
}
bool acceptsQuickGameSource(const fs::path &source)
{
    std::error_code error;
    if (!source.is_absolute())
        return false;

    auto extension = source.extension().wstring();
    std::transform(extension.begin(), extension.end(), extension.begin(), towlower);
    return extension == L".iso" && fs::is_regular_file(source, error);
}
void startQuickInstall()
{
    if (running) return;
    try
    {
        auto values = collectInputs();
        if (needsPreviousOutput && values[PreviousOutputField].empty())
        {
            SetWindowTextW(statusLabel, L"Choose the folder of an existing Venom build.");
            SetFocus(browseButtons[PreviousOutputField]);
            return;
        }
        const fs::path game(values[RetailField]);
        if (!acceptsQuickGameSource(game))
        {
            SetWindowTextW(statusLabel, L"Drop an Xbox .iso here or choose an extracted game folder.");
            SetFocus(browseButtons[RetailField]);
            return;
        }
        auto interpreter = !localInstallPython.empty() ? localInstallPython :
            values[PythonField].empty() ? systemPowerShell() : fs::path(values[PythonField]);
        if (localInstallPython.empty() && values[PythonField].empty() && !fs::is_regular_file(interpreter))
            interpreter = findInstalledPython();
        if (interpreter.empty() || !fs::is_regular_file(interpreter))
        {
            SetWindowTextW(statusLabel, L"Setup could not start its runtime. Choose Python in Advanced.");
            return;
        }
        SYSTEMTIME now;
        GetLocalTime(&now);
        wchar_t suffix[64];
        swprintf_s(suffix, L"Venom-%04u%02u%02u-%02u%02u%02u-%03u", now.wYear, now.wMonth,
                   now.wDay, now.wHour, now.wMinute, now.wSecond, now.wMilliseconds);
        const fs::path parent(windowText(installLocationInput));
        if (!parent.is_absolute() || (fs::exists(parent) && !fs::is_directory(parent)))
        {
            SetWindowTextW(statusLabel, L"Choose a folder for the install location.");
            SetFocus(installLocationInput);
            return;
        }
        const auto output = parent.lexically_normal() / suffix;
        if (hasReparsePoint(output) || fs::exists(output))
            throw std::runtime_error("Choose a private output folder in Advanced.");
        values[PythonField] = interpreter.wstring();
        values[OutputField] = output.wstring();
        SetWindowTextW(fieldInputs[OutputField], output.c_str());
        activeOutput = output;
        running = true;
        simpleStarted = true;
        needsBuildTools = false;
        setInputsEnabled(false);
        SetWindowPos(mainWindow, nullptr, 0, 0, 740, 575,
                     SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
        SetWindowTextW(statusLabel, localInstallPython.empty()
            ? L"Checking the ISO and preparing the local copy…"
            : L"Checking your ISO and copying the existing build…");
        appendLog(L"\r\n[Checking game files and preparing the installation.]\r\n");
        ownedBuildThread = std::thread(runPipeline, values, false, false, true);
    }
    catch (const std::exception &error)
    {
        running = false;
        setInputsEnabled(true);
        SetWindowTextW(statusLabel, decodeUtf8(error.what()).c_str());
    }
}
HWND createControl(const wchar_t *className, const wchar_t *caption, DWORD style, int id = 0)
{
    auto controlHandle = CreateWindowExW(
        style & WS_BORDER ? WS_EX_CLIENTEDGE : 0, className, caption, WS_CHILD | WS_VISIBLE | style,
        0, 0, 1, 1, mainWindow, (HMENU)(INT_PTR)id, GetModuleHandleW(nullptr), nullptr);
    SendMessageW(controlHandle, WM_SETFONT, (WPARAM)uiFont, TRUE);
    return controlHandle;
}
void layoutControls(int width, int height)
{
    const bool basic = simpleMode && !uiSelfTest;
    for (int i = 0; i < FieldCount; ++i)
    {
        const bool showField = !basic || i == RetailField || (i == PreviousOutputField && needsPreviousOutput);
        const int visibility = showField ? SW_SHOW : SW_HIDE;
        ShowWindow(fieldInputs[i], visibility);
        ShowWindow(fieldLabels[i], visibility);
        if (browseButtons[i]) ShowWindow(browseButtons[i], visibility);
    }
    for (HWND control : {reviewButton, loadButton, rememberBox, resumeBox})
        ShowWindow(control, basic ? SW_HIDE : SW_SHOW);
    ShowWindow(advancedButton, SW_HIDE);
    ShowWindow(progressLog, !basic || simpleStarted ? SW_SHOW : SW_HIDE);
    for (HWND control : {installLocationLabel, installLocationInput, installLocationBrowse})
        ShowWindow(control, basic ? SW_SHOW : SW_HIDE);
    ShowWindow(buildToolsButton, basic && needsBuildTools ? SW_SHOW : SW_HIDE);
    std::error_code error;
    ShowWindow(openButton, !basic || fs::is_directory(activeOutput, error) ? SW_SHOW : SW_HIDE);
    if (basic)
    {
        MoveWindow(introductionLabel, 24, 20, width - 48, 56, TRUE);
        MoveWindow(fieldLabels[RetailField], 24, 88, width - 48, 22, TRUE);
        MoveWindow(fieldInputs[RetailField], 24, 114, width - 160, 28, TRUE);
        MoveWindow(browseButtons[RetailField], width - 125, 114, 101, 28, TRUE);
        MoveWindow(installLocationLabel, 24, 156, width - 48, 22, TRUE);
        MoveWindow(installLocationInput, 24, 182, width - 160, 28, TRUE);
        MoveWindow(installLocationBrowse, width - 125, 182, 101, 28, TRUE);
        int actionsY = 230;
        if (needsPreviousOutput)
        {
            MoveWindow(fieldLabels[PreviousOutputField], 24, 224, width - 48, 22, TRUE);
            MoveWindow(fieldInputs[PreviousOutputField], 24, 250, width - 160, 28, TRUE);
            MoveWindow(browseButtons[PreviousOutputField], width - 125, 250, 101, 28, TRUE);
            actionsY += 68;
        }
        MoveWindow(buildButton, 24, actionsY, 118, 32, TRUE);
        MoveWindow(advancedButton, 154, actionsY, 110, 32, TRUE);
        MoveWindow(buildToolsButton, 276, actionsY, 166, 32, TRUE);
        MoveWindow(openButton, width - 184, actionsY, 160, 32, TRUE);
        MoveWindow(statusLabel, 24, actionsY + 49, width - 48, 42, TRUE);
        MoveWindow(progressLog, 24, actionsY + 95, width - 48, std::max(70, height - actionsY - 119), TRUE);
        return;
    }
    MoveWindow(introductionLabel, 16, 12, width - 32, 40, TRUE);
    int y = 60, labelWidth = 330, editWidth = std::max(200, width - 460);
    for (int fieldIndex = 0; fieldIndex < FieldCount; ++fieldIndex)
    {
        MoveWindow(fieldLabels[fieldIndex], 16, y + 4, labelWidth - 6, 24, TRUE);
        MoveWindow(fieldInputs[fieldIndex], 346, y, editWidth, 25, TRUE);
        if (browseButtons[fieldIndex])
        {
            MoveWindow(browseButtons[fieldIndex], 352 + editWidth, y, 88, 25, TRUE);
        }
        y += 30;
    }
    MoveWindow(rememberBox, 16, y, width - 32, 24, TRUE);
    y += 26;
    MoveWindow(resumeBox, 16, y, width - 32, 24, TRUE);
    y += 32;
    MoveWindow(loadButton, 16, y, 155, 30, TRUE);
    MoveWindow(reviewButton, 180, y, 150, 30, TRUE);
    MoveWindow(buildButton, 339, y, 175, 30, TRUE);
    MoveWindow(openButton, 523, y, 170, 30, TRUE);
    MoveWindow(advancedButton, 705, y, 135, 30, TRUE);
    y += 38;
    MoveWindow(statusLabel, 16, y, width - 32, 36, TRUE);
    y += 40;
    MoveWindow(progressLog, 16, y, width - 32, std::max(80, height - y - 16), TRUE);
}
LRESULT CALLBACK windowProcedure(HWND windowHandle, UINT message, WPARAM wordParameter,
                                 LPARAM longParameter)
{
    switch (message)
    {
    case WM_CREATE: {
        mainWindow = windowHandle;
        uiFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        introductionLabel =
            createControl(L"STATIC",
                          L"Build Fight Night Champion Venom from your owned retail game, exact "
                          L"TU2 and DLC.\r\nChoose private output with sufficient drive space. "
                          L"This installer never launches the game.",
                          0);
        for (int fieldIndex = 0; fieldIndex < FieldCount; ++fieldIndex)
        {
            fieldLabels[fieldIndex] = createControl(L"STATIC", fields[fieldIndex].label, 0);
            fieldInputs[fieldIndex] =
                createControl(L"EDIT", fields[fieldIndex].initial,
                              WS_TABSTOP | WS_BORDER | ES_AUTOHSCROLL, 100 + fieldIndex);
            SendMessageW(fieldInputs[fieldIndex], EM_SETLIMITTEXT, 32760, 0);
            if (fields[fieldIndex].hasBrowseButton)
            {
                browseButtons[fieldIndex] = createControl(
                    L"BUTTON", fieldIndex == OutputField ? L"Parent..." : L"Browse...",
                    WS_TABSTOP | BS_PUSHBUTTON, 200 + fieldIndex);
            }
        }
        installLocationLabel = createControl(L"STATIC", L"Install location", 0);
        installLocationInput = createControl(L"EDIT",
            (packageRoot.parent_path() / L"Venom Builds").c_str(), WS_TABSTOP | WS_BORDER | ES_AUTOHSCROLL);
        SendMessageW(installLocationInput, EM_SETLIMITTEXT, 32760, 0);
        installLocationBrowse = createControl(L"BUTTON", L"Browse...", WS_TABSTOP, InstallLocationBrowseId);
        rememberBox = createControl(
            L"BUTTON",
            L"Remember input paths only inside selected output after the pipeline creates it",
            WS_TABSTOP | BS_AUTOCHECKBOX, RememberInputsId);
        SendMessageW(rememberBox, BM_SETCHECK, BST_CHECKED, 0);
        resumeBox = createControl(L"BUTTON",
                                  L"Resume exact existing output (completed, hash-verified stages "
                                  L"only; incomplete stages preserved)",
                                  WS_TABSTOP | BS_AUTOCHECKBOX, ResumeBuildId);
        loadButton = createControl(L"BUTTON", L"Load previous inputs...", WS_TABSTOP, LoadInputsId);
        reviewButton =
            createControl(L"BUTTON", L"Review exact command", WS_TABSTOP, ReviewCommandId);
        buildButton = createControl(L"BUTTON", L"Build and install", WS_TABSTOP | BS_DEFPUSHBUTTON,
                                    BuildCommandId);
        openButton = createControl(L"BUTTON", L"Open output folder", WS_TABSTOP, OpenOutputId);
        advancedButton = createControl(L"BUTTON", L"Advanced", WS_TABSTOP, AdvancedId);
        buildToolsButton = createControl(L"BUTTON", L"Get C++ Build Tools", WS_TABSTOP, BuildToolsId);
        if (!uiSelfTest)
        {
            SetWindowTextW(introductionLabel,
                L"Drop an Xbox disc image (.iso) here or click Browse to select it. "
                L"Setup installs the existing local build.");
            SetWindowTextW(fieldLabels[RetailField], L"Game source");
            SetWindowTextW(fieldLabels[PreviousOutputField], L"Existing Venom build (folder)");
            SetWindowTextW(buildButton, L"Install");
        }
        statusLabel = createControl(L"STATIC",
                                    L"Select inputs, then review command. Exact owned-media "
                                    L"validation is performed by the backend.",
                                    0);
        progressLog = createControl(L"EDIT", L"",
                                    WS_BORDER | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE |
                                        ES_READONLY | ES_AUTOVSCROLL | ES_AUTOHSCROLL);
        SendMessageW(progressLog, EM_SETLIMITTEXT, 260000, 0);
        if (!uiSelfTest)
            DragAcceptFiles(windowHandle, TRUE);
        if (!uiSelfTest)
            SetWindowTextW(statusLabel, localInstallPython.empty()
                ? L"Install files go into Venom Builds beside the extracted installer."
                : L"Choose the install location, then click Install.");
        RECT bounds{};
        GetClientRect(windowHandle, &bounds);
        layoutControls(bounds.right, bounds.bottom);
        return 0;
    }
    case WM_DROPFILES: {
        const auto drop = reinterpret_cast<HDROP>(wordParameter);
        std::wstring source;
        if (simpleMode && !uiSelfTest && !running && DragQueryFileW(drop, 0xFFFFFFFF, nullptr, 0) == 1)
        {
            const auto length = DragQueryFileW(drop, 0, nullptr, 0);
            source.resize(length + 1);
            source.resize(DragQueryFileW(drop, 0, source.data(), length + 1));
        }
        DragFinish(drop);
        if (simpleMode && !uiSelfTest && !running)
        {
            if (acceptsQuickGameSource(source))
            {
                SetWindowTextW(fieldInputs[RetailField], source.c_str());
                SetWindowTextW(statusLabel, L"Game source selected. Choose a location, then click Install.");
            }
            else
                SetWindowTextW(statusLabel, L"Drop one Xbox .iso or one extracted game folder.");
        }
        return 0;
    }
    case WM_SIZE:
        layoutControls(LOWORD(longParameter), HIWORD(longParameter));
        return 0;
    case WM_GETMINMAXINFO:
        ((MINMAXINFO *)longParameter)->ptMinTrackSize = simpleMode && !uiSelfTest ? POINT{650, 470} : POINT{960, 800};
        return 0;
    case WM_COMMAND: {
        if (uiSelfTest)
        {
            // The scenario drives only the shared handler, never dialogs or Explorer.
            return 0;
        }
        int controlId = LOWORD(wordParameter);
        if (controlId != BuildCommandId && controlId != 200 + RetailField && controlId != InstallLocationBrowseId && controlId != 200 + PreviousOutputField && controlId != OpenOutputId)
            return 0;
        if (controlId == InstallLocationBrowseId && simpleMode && !running)
        {
            const auto folder = selectPath(true, L"Choose the install location");
            if (!folder.empty())
                SetWindowTextW(installLocationInput, folder.c_str());
            return 0;
        }
        if (controlId >= 200 && controlId < 200 + FieldCount)
        {
            int fieldIndex = controlId - 200;
            const bool isoPicker = simpleMode && fieldIndex == RetailField;
            const fs::path current = windowText(fieldInputs[RetailField]);
            auto selectedPath = isoPicker
                ? selectPath(false, L"Choose your Xbox disc image (.iso)", true, current.parent_path())
                : selectPath(fields[fieldIndex].folder,
                    fieldIndex == OutputField ? L"Choose parent for a NEW private Venom build folder"
                                              : fields[fieldIndex].label);
            if (!selectedPath.empty())
            {
                if (fieldIndex == OutputField)
                {
                    SYSTEMTIME timestamp;
                    GetLocalTime(&timestamp);
                    wchar_t suffix[80];
                    wsprintfW(suffix, L"Venom-build-%04u%02u%02u-%02u%02u%02u", timestamp.wYear,
                              timestamp.wMonth, timestamp.wDay, timestamp.wHour, timestamp.wMinute,
                              timestamp.wSecond);
                    selectedPath = (fs::path(selectedPath) / suffix).wstring();
                }
                SetWindowTextW(fieldInputs[fieldIndex], selectedPath.c_str());

            }
            return 0;
        }
        if (controlId == ReviewCommandId)
        {
            reviewAndStart(false);
        }
        else if (controlId == BuildCommandId)
        {
            if (simpleMode) startQuickInstall();
            else reviewAndStart(true);
        }
        else if (controlId == BuildToolsId && !running)
        {
            ShellExecuteW(windowHandle, L"open",
                L"https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022",
                nullptr, nullptr, SW_SHOWNORMAL);
        }
        else if (controlId == AdvancedId && !running)
        {
            simpleMode = !simpleMode;
            SetWindowTextW(advancedButton, simpleMode ? L"Advanced" : L"Simple setup");
            SetWindowTextW(buildButton, simpleMode ? L"Install" : L"Build and install");
            SetWindowTextW(fieldLabels[RetailField], simpleMode ? L"Game source" : fields[RetailField].label);
            SetWindowTextW(introductionLabel, simpleMode
                ? L"Drop an Xbox disc image (.iso) here or click Browse to select it."
                : L"Choose the owned files, build tools and installation output.");
            SetWindowPos(mainWindow, nullptr, 0, 0, simpleMode ? 740 : 1120,
                         simpleMode ? (simpleStarted ? 575 : 400) : 900, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
            RECT bounds{};
            GetClientRect(windowHandle, &bounds);
            layoutControls(bounds.right, bounds.bottom);
        }
        else if (controlId == LoadInputsId)
        {
            loadInputs();
        }
        else if (controlId == OpenOutputId)
        {
            auto selectedPath =
                simpleMode ? activeOutput : fs::path(windowText(fieldInputs[OutputField]));
            if (!selectedPath.empty() && fs::is_directory(selectedPath))
            {
                ShellExecuteW(windowHandle, L"open", selectedPath.c_str(), nullptr, nullptr,
                              SW_SHOWNORMAL);
            }
            else
            {
                MessageBoxW(windowHandle, L"Selected output does not exist yet.", L"Private output",
                            MB_OK);
            }
        }
        return 0;
    }
    case LogMessage: {
        auto logText = (std::wstring *)longParameter;
        appendLog(*logText);
        delete logText;
        return 0;
    }
    case BuildDoneMessage:
        if (ownedBuildThread.joinable())
        {
            ownedBuildThread.join();
        }
        running = false;
        setInputsEnabled(true);
        if (simpleMode && !uiSelfTest)
        {
            needsBuildTools = wordParameter == 78;
            if (wordParameter == PreviousOutputRequiredExit)
            {
                needsPreviousOutput = true;
                appendLog(L"\r\n[Choose the previous install's build folder. "
                          L"Setup paused before copying any saves.]\r\n");
            }
            SetWindowTextW(statusLabel, wordParameter == 0
                ? L"Installation complete. Open the output folder to find your game launcher."
                : wordParameter == PreviousOutputRequiredExit
                    ? L"Choose the previous Venom install with your save, then click Install."
                    : needsBuildTools ? L"Install Microsoft C++ Build Tools, then click Install again."
                                      : L"Setup needs attention. See the details below.");
            RECT bounds{};
            GetClientRect(windowHandle, &bounds);
            layoutControls(bounds.right, bounds.bottom);
        }
        else
        {
            SetWindowTextW(statusLabel, wordParameter == 0
                ? L"Build and install complete."
                : L"Build or install failed. See the details below.");
        }
        if (uiSelfTest)
        {
            finishUiSelfTest(static_cast<DWORD>(wordParameter));
        }
        return 0;
    case SelfTestStartMessage:
        if (uiSelfTest)
        {
            beginUiSelfTest();
        }
        return 0;
    case WM_TIMER:
        if (uiSelfTest && wordParameter == SelfTestTimerId &&
            GetTickCount64() >= uiSelfTest->deadline)
        {
            uiSelfTest->timedOut = true;
            uiSelfTest->failure = "Finite self-test deadline reached";
            selfTestCancelRequested.store(true);
            KillTimer(windowHandle, SelfTestTimerId);
        }
        return 0;
    case WM_CLOSE:
        if (uiSelfTest)
        {
            uiSelfTest->failure = "Own self-test window closed before completion";
            if (running)
            {
                selfTestCancelRequested.store(true);
            }
            else
            {
                finishUiSelfTest(ERROR_CANCELLED);
            }
            return 0;
        }
        if (running)
        {
            MessageBoxW(windowHandle,
                        L"Build running. Keep this window open until it exits. Logs and incomplete "
                        L"stages are preserved in selected output.",
                        L"Build running", MB_OK);
            return 0;
        }
        DestroyWindow(windowHandle);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(uiSelfTest ? uiSelfTest->exitCode : 0);
        return 0;
    }
    return DefWindowProcW(windowHandle, message, wordParameter, longParameter);
}
#ifdef VENOM_STATIC_TEST
int wmain(int argumentCount, wchar_t **argumentValues)
{
    if (argumentCount == 2 && std::wstring(argumentValues[1]) == L"--job-test-child")
    {
        Sleep(INFINITE);
        return 9;
    }
    if (argumentCount == 3 && std::wstring(argumentValues[1]) == L"--iso-picker-config-test")
    {
        if (FAILED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED))) return 45;
        IFileDialog *dialog = nullptr;
        const HRESULT created = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER,
                                                 IID_PPV_ARGS(&dialog));
        bool passed = false;
        if (SUCCEEDED(created))
        {
            FILEOPENDIALOGOPTIONS options{};
            UINT filter = 0;
            passed = SUCCEEDED(configureIsoPicker(dialog, argumentValues[2])) &&
                SUCCEEDED(dialog->GetOptions(&options)) && !(options & FOS_PICKFOLDERS) &&
                SUCCEEDED(dialog->GetFileTypeIndex(&filter)) && filter == 1;
            dialog->Release();
        }
        CoUninitialize();
        if (!passed) return 46;
        std::puts("PASS: Windows file dialog configured with ISO filter and file selection.");
        return 0;
    }
    if ((argumentCount == 3 || argumentCount == 7) &&
        std::wstring(argumentValues[1]) == L"--iso-drop-control-test")
    {
        if (argumentCount == 7)
            packageRoot = argumentValues[3];
        const auto instance = GetModuleHandleW(nullptr);
        WNDCLASSEXW wc{};
        wc.cbSize = sizeof(wc);
        wc.lpfnWndProc = windowProcedure;
        wc.hInstance = instance;
        wc.lpszClassName = L"VenomIsoDropControlTest";
        if (!RegisterClassExW(&wc)) return 40;
        const auto window = CreateWindowExW(0, wc.lpszClassName, L"Fixture", WS_OVERLAPPEDWINDOW,
                                            0, 0, 740, 525, nullptr, nullptr, instance, nullptr);
        if (!window) return 41;
        const std::wstring source = argumentValues[2];
        const auto bytes = sizeof(DROPFILES) + (source.size() + 2) * sizeof(wchar_t);
        const auto memory = GlobalAlloc(GHND, bytes);
        if (!memory) return 42;
        auto *drop = static_cast<DROPFILES *>(GlobalLock(memory));
        if (!drop) { GlobalFree(memory); return 43; }
        drop->pFiles = sizeof(DROPFILES);
        drop->fWide = TRUE;
        auto *path = reinterpret_cast<wchar_t *>(reinterpret_cast<char *>(drop) + sizeof(DROPFILES));
        std::copy(source.begin(), source.end(), path);
        GlobalUnlock(memory);
        SendMessageW(window, WM_DROPFILES, reinterpret_cast<WPARAM>(memory), 0);
        const bool selected = windowText(fieldInputs[RetailField]) == source &&
            windowText(statusLabel).find(L"Game source selected") != std::wstring::npos;
        if (argumentCount == 7 && selected)
        {
            const fs::path location = argumentValues[4];
            if (windowText(buildButton) != L"Install") return 47;
            SetWindowTextW(installLocationInput, location.c_str());
            SetWindowTextW(fieldInputs[PreviousOutputField], argumentValues[6]);
            SendMessageW(window, WM_COMMAND, MAKEWPARAM(BuildCommandId, BN_CLICKED), 0);
            const auto deadline = GetTickCount64() + 120000;
            while (running && GetTickCount64() < deadline)
            {
                MSG message{};
                while (PeekMessageW(&message, window, 0, 0, PM_REMOVE))
                    DispatchMessageW(&message);
                Sleep(10);
            }
            if (running) std::quick_exit(48);
            if (lastChildCode.load() != 0 || !lastJobAssigned.load() || lastMonitorFailed.load() ||
                activeOutput.parent_path() != location ||
                !fs::is_regular_file(activeOutput / L"build-install-receipt.json") ||
                windowText(statusLabel).find(L"Installation complete") == std::wstring::npos)
            {
                std::fprintf(stderr, "%s\n", strictUtf8(windowText(progressLog)).c_str());
                return 49;
            }
            std::printf("TEST_OUTPUT=%s\n", jsonString(strictUtf8(activeOutput.wstring())).c_str());
        }
        DestroyWindow(window);
        if (!selected) return 44;
        std::puts("PASS: native Setup accepted an ISO drop without starting the build.");
        return 0;
    }
    if (argumentCount == 2 && std::wstring(argumentValues[1]) == L"--save-picker-control-test")
    {
        auto instance = GetModuleHandleW(nullptr);
        WNDCLASSEXW wc{};
        wc.cbSize = sizeof(wc);
        wc.lpfnWndProc = windowProcedure;
        wc.hInstance = instance;
        wc.lpszClassName = L"VenomSavePickerControlTest";
        if (!RegisterClassExW(&wc)) return 30;
        const auto window = CreateWindowExW(0, wc.lpszClassName, L"Fixture", WS_OVERLAPPEDWINDOW,
                                            0, 0, 740, 525, nullptr, nullptr, instance, nullptr);
        if (!window) return 31;
        const auto visible = [](HWND control) {
            return (GetWindowLongPtrW(control, GWL_STYLE) & WS_VISIBLE) != 0;
        };
        const auto previous = fieldInputs[PreviousOutputField];
        const auto browse = browseButtons[PreviousOutputField];
        const bool initialOnlyGame = visible(fieldInputs[RetailField]) && !visible(previous) && !visible(browse);
        SendMessageW(window, BuildDoneMessage, PreviousOutputRequiredExit, 0);
        const bool ambiguityShowsPicker = visible(previous) && visible(browse) &&
            windowText(statusLabel).find(L"Choose the previous Venom install") != std::wstring::npos &&
            windowText(progressLog).find(L"Setup paused before copying any saves.") != std::wstring::npos;
        SetWindowTextW(previous, L"D:/Fixture previous build");
        SendMessageW(window, BuildDoneMessage, 1, 0);
        const bool retryRetainsChoice = visible(previous) &&
            windowText(previous) == L"D:/Fixture previous build";
        SendMessageW(window, BuildDoneMessage, 0, 0);
        const bool successResetsPicker = !visible(previous) && !visible(browse) && windowText(previous).empty();
        DestroyWindow(window);
        if (!initialOnlyGame || !ambiguityShowsPicker || !retryRetainsChoice || !successResetsPicker)
            return 32;
        std::puts("PASS: hidden Quick Setup controls reveal the prior install picker, retain selection on retry, and reset on success.");
        return 0;
    }
    if (argumentCount == 3 && std::wstring(argumentValues[1]) == L"--describe-ui-self-test")
    {
        try
        {
            fs::path path(argumentValues[2]);
            selfTestRequire(privateQaPath(path.parent_path()),
                            "Descriptor requires private E QA data");
            auto scenario = parseSelfTestScenario(readScenario(path));
            packageRoot = scenario.sourceRoot;
            auto command =
                strictUtf8(makeCommandLine(makePipelineArguments(scenario.values, true)));
            auto description = std::string("{\"commandLine\":") + jsonString(command) +
                               ",\"commandSha256\":" + jsonString(sha256Text(command)) + "}\n";
            std::fwrite(description.data(), 1, description.size(), stdout);
            return 0;
        }
        catch (const std::exception &error)
        {
            std::fprintf(stderr, "%s\n", error.what());
            return 20;
        }
    }
    unsigned diagnosticCases = 0;
    try
    {
        diagnosticCases = selfTestScenarioRegressions();
    }
    catch (const std::exception &error)
    {
        std::fprintf(stderr, "Diagnostic schema regression: %s\n", error.what());
        return 21;
    }
    // Exercise actual Windows argument parsing, including shell metacharacters as data.
    std::vector<std::wstring> cases = {L"",
                                       L"plain",
                                       L"E:\\Private output\\",
                                       L"a\"b",
                                       L"\\\"",
                                       L"a & whoami | < > ^ %PATH% $(test)",
                                       L"\u0394\u4e2d\u6587",
                                       L"\\\\server\\folder with spaces\\"};
    for (unsigned caseIndex = 0; caseIndex < 240; ++caseIndex)
    {
        std::wstring argument;
        for (unsigned characterIndex = 0; characterIndex < caseIndex % 17; ++characterIndex)
        {
            argument += L"ab \\\"&"[(caseIndex * 37 + characterIndex * 13) % 6];
        }
        cases.push_back(argument);
    }
    for (const auto &argument : cases)
    {
        auto commandLine = L"self.exe " + quoteArgument(argument);
        int count = 0;
        auto parsedArguments = CommandLineToArgvW(commandLine.c_str(), &count);
        if (!parsedArguments || count != 2 || parsedArguments[1] != argument)
        {
            if (parsedArguments)
            {
                LocalFree(parsedArguments);
            }
            return 1;
        }
        LocalFree(parsedArguments);
    }
    if (!isSameOrInside(L"E:/private/build", L"e:/PRIVATE") ||
        isSameOrInside(L"E:/private-other", L"E:/private"))
    {
        return 2;
    }
    packageRoot = L"E:/source package";
    std::vector<std::wstring> inputValues(FieldCount, L"x");
    inputValues[AptOverrideField] = L"";
    inputValues[InstallField] = L"";
    inputValues[PreviousOutputField] = L"";
    auto parsedArguments = makePipelineArguments(inputValues, false);
    if (std::find(parsedArguments.begin(), parsedArguments.end(), L"--apt-tools-root") !=
            parsedArguments.end() ||
        std::find(parsedArguments.begin(), parsedArguments.end(), L"--resume") !=
            parsedArguments.end())
    {
        return 3;
    }
    if (std::find(parsedArguments.begin(), parsedArguments.end(), L"--previous-output") != parsedArguments.end())
        return 8;
    inputValues[PreviousOutputField] = L"E:/previous Venom output";
    auto upgradeArguments = makePipelineArguments(inputValues, false);
    auto previousOption = std::find(upgradeArguments.begin(), upgradeArguments.end(), L"--previous-output");
    if (previousOption == upgradeArguments.end() || std::next(previousOption) == upgradeArguments.end() ||
        *std::next(previousOption) != inputValues[PreviousOutputField])
        return 9;
    inputValues[PythonField] = L"D:/fixture with spaces/python.exe";
    inputValues[PreviousOutputField] = L"D:/Player's saves & settings/installed";
    const auto quickUpgrade = makeQuickArguments(inputValues);
    const auto quickOption = std::find(quickUpgrade.begin(), quickUpgrade.end(), L"--previous-output");
    if (quickOption == quickUpgrade.end() || std::next(quickOption) == quickUpgrade.end() ||
        *std::next(quickOption) != inputValues[PreviousOutputField] ||
        powerShellLiteral(inputValues[PreviousOutputField]) != L"'D:/Player''s saves & settings/installed'")
        return 24;
    inputValues[PreviousOutputField].clear();
    const auto quickDefault = makeQuickArguments(inputValues);
    if (std::find(quickDefault.begin(), quickDefault.end(), L"--previous-output") != quickDefault.end())
        return 25;
    auto resumedArguments = makePipelineArguments(inputValues, true);
    if (resumedArguments.back() != L"--resume")
    {
        return 4;
    }
    inputValues[InstallField] = L"E:/private/build/installed-new";
    auto installArguments = makePipelineArguments(inputValues, true);
    if (std::find(installArguments.begin(), installArguments.end(), L"--install-root") ==
        installArguments.end())
    {
        return 5;
    }
    // Verify kill-on-close containment against this console test's own suspended child.
    // No GUI, game, SDK compiler, source media, or unrelated process participates.
    HANDLE job = createOwnedBuildJob();
    wchar_t executablePath[32768];
    GetModuleFileNameW(nullptr, executablePath, 32768);
    auto childCommand = makeCommandLine({executablePath, L"--job-test-child"});
    STARTUPINFOW startup{};
    startup.cb = sizeof(startup);
    PROCESS_INFORMATION child{};
    if (!CreateProcessW(executablePath, childCommand.data(), nullptr, nullptr, FALSE,
                        CREATE_NO_WINDOW | CREATE_SUSPENDED, nullptr, nullptr, &startup, &child))
    {
        CloseHandle(job);
        return 6;
    }
    if (!AssignProcessToJobObject(job, child.hProcess))
    {
        TerminateProcess(child.hProcess, ERROR_GEN_FAILURE);
        CloseHandle(job);
        CloseHandle(child.hThread);
        CloseHandle(child.hProcess);
        return 7;
    }
    ResumeThread(child.hThread);
    CloseHandle(child.hThread);
    CloseHandle(job);
    DWORD waitStatus = WaitForSingleObject(child.hProcess, 5000);
    DWORD childExitCode = STILL_ACTIVE;
    GetExitCodeProcess(child.hProcess, &childExitCode);
    if (waitStatus != WAIT_OBJECT_0 || childExitCode == STILL_ACTIVE)
    {
        TerminateProcess(child.hProcess, ERROR_GEN_FAILURE);
        CloseHandle(child.hProcess);
        return 8;
    }
    CloseHandle(child.hProcess);
    std::printf(
        "PASS 248 Windows argument round trips; %u diagnostic schema cases; owned-job cleanup\n",
        diagnosticCases);
    return 0;
}
#else
int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int show)
{
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    wchar_t executablePath[32768];
    GetModuleFileNameW(nullptr, executablePath, 32768);
    packageRoot = fs::path(executablePath).parent_path();
    int argumentCount = 0;
    auto arguments = CommandLineToArgvW(GetCommandLineW(), &argumentCount);
    try
    {
        selfTestRequire(arguments != nullptr, "Cannot parse installer arguments");
        if ((argumentCount == 4 || argumentCount == 5) && std::wstring(arguments[1]) == L"--local-copy")
        {
            localInstallPython = arguments[2];
            localInstallSource = arguments[3];
            if (argumentCount == 5) localInstallLocation = arguments[4];
            selfTestRequire(fs::is_regular_file(localInstallPython), "The preview Python executable is missing");
        }
        else if (argumentCount != 1)
        {
            selfTestRequire(argumentCount == 5 && std::wstring(arguments[1]) == L"--ui-self-test" &&
                                std::wstring(arguments[3]) == L"--scenario-sha256",
                            "Use --ui-self-test <scenario.ini> --scenario-sha256 <SHA256>");
            configureUiSelfTest(arguments[2], strictUtf8(arguments[4]), executablePath);
        }
        LocalFree(arguments);
    }
    catch (const std::exception &error)
    {
        if (arguments)
            LocalFree(arguments);
        DWORD written = 0;
        auto text = std::string(error.what()) + "\n";
        WriteFile(GetStdHandle(STD_ERROR_HANDLE), text.data(), static_cast<DWORD>(text.size()),
                  &written, nullptr);
        CoUninitialize();
        return 2;
    }
    WNDCLASSEXW windowClass{};
    windowClass.cbSize = sizeof(windowClass);
    windowClass.lpfnWndProc = windowProcedure;
    windowClass.hInstance = instance;
    windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    windowClass.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    windowClass.lpszClassName = L"VenomOwnedBuildInstaller";
    RegisterClassExW(&windowClass);
    HWND windowHandle = CreateWindowExW(
        uiSelfTest ? WS_EX_NOACTIVATE : 0, windowClass.lpszClassName,
        L"Fight Night Champion Venom - Install Existing Build", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
        CW_USEDEFAULT, uiSelfTest ? 1120 : 740, uiSelfTest ? 900 : 470,
        nullptr, nullptr, instance, nullptr);
    if (!windowHandle)
    {
        return 1;
    }
    ShowWindow(windowHandle, uiSelfTest ? SW_SHOWNOACTIVATE : show);
    if (!localInstallPython.empty())
    {
        SetWindowTextW(fieldInputs[RetailField], localInstallSource.c_str());
        if (!localInstallLocation.empty()) SetWindowTextW(installLocationInput, localInstallLocation.c_str());
    }
    if (uiSelfTest)
    {
        if (!PostMessageW(windowHandle, SelfTestStartMessage, 0, 0))
        {
            uiSelfTest->failure = "Cannot queue own self-test start";
            finishUiSelfTest(ERROR_GEN_FAILURE);
        }
    }
    MSG message{};
    while (GetMessageW(&message, nullptr, 0, 0) > 0)
    {
        if (!IsDialogMessageW(windowHandle, &message))
        {
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
    }
    CoUninitialize();
    return (int)message.wParam;
}
#endif
