#pragma once
// Explicit app-owned diagnostics. This header never locates or controls another window.
#include <atomic>
#include <bcrypt.h>
#include <cstdint>
#include <cstdio>
#include <iomanip>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <stdexcept>

constexpr UINT SelfTestStartMessage = WM_APP + 3;
constexpr UINT_PTR SelfTestTimerId = 71;

void selfTestRequire(bool condition, const char *message)
{
    if (!condition)
    {
        throw std::runtime_error(message);
    }
}

std::string strictUtf8(const std::wstring &value)
{
    if (value.empty())
    {
        return {};
    }
    int count = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, value.data(),
                                    static_cast<int>(value.size()), nullptr, 0, nullptr, nullptr);
    selfTestRequire(count > 0, "Invalid UTF16 diagnostic value");
    std::string result(count, '\0');
    selfTestRequire(WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, value.data(),
                                        static_cast<int>(value.size()), result.data(), count,
                                        nullptr, nullptr) == count,
                    "Cannot encode diagnostic value");
    return result;
}

std::wstring strictWide(const std::string &value)
{
    if (value.empty())
    {
        return {};
    }
    int count = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value.data(),
                                    static_cast<int>(value.size()), nullptr, 0);
    selfTestRequire(count > 0, "Invalid UTF8 scenario");
    std::wstring result(count, L'\0');
    selfTestRequire(MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value.data(),
                                        static_cast<int>(value.size()), result.data(),
                                        count) == count,
                    "Cannot decode scenario");
    return result;
}

std::string sha256Stream(std::istream &stream)
{
    BCRYPT_ALG_HANDLE algorithm = nullptr;
    BCRYPT_HASH_HANDLE hash = nullptr;
    selfTestRequire(BCryptOpenAlgorithmProvider(&algorithm, BCRYPT_SHA256_ALGORITHM, nullptr, 0) >=
                        0,
                    "Cannot open diagnostic SHA256 provider");
    if (BCryptCreateHash(algorithm, &hash, nullptr, 0, nullptr, 0, 0) < 0)
    {
        BCryptCloseAlgorithmProvider(algorithm, 0);
        throw std::runtime_error("Cannot create diagnostic SHA256 state");
    }
    try
    {
        char buffer[65536];
        while (stream)
        {
            stream.read(buffer, sizeof(buffer));
            auto count = stream.gcount();
            if (count)
            {
                selfTestRequire(BCryptHashData(hash, reinterpret_cast<PUCHAR>(buffer),
                                               static_cast<ULONG>(count), 0) >= 0,
                                "Diagnostic SHA256 update failed");
            }
        }
        selfTestRequire(stream.eof(), "Diagnostic hash read failed");
        unsigned char result[32];
        selfTestRequire(BCryptFinishHash(hash, result, sizeof(result), 0) >= 0,
                        "Diagnostic SHA256 finish failed");
        BCryptDestroyHash(hash);
        hash = nullptr;
        BCryptCloseAlgorithmProvider(algorithm, 0);
        algorithm = nullptr;
        std::ostringstream encoded;
        for (auto byte : result)
        {
            encoded << std::hex << std::setfill('0') << std::setw(2) << static_cast<unsigned>(byte);
        }
        return encoded.str();
    }
    catch (...)
    {
        if (hash)
        {
            BCryptDestroyHash(hash);
        }
        if (algorithm)
        {
            BCryptCloseAlgorithmProvider(algorithm, 0);
        }
        throw;
    }
}

std::string sha256File(const fs::path &path)
{
    std::ifstream stream(path, std::ios::binary);
    selfTestRequire(bool(stream), "Cannot open diagnostic hash input");
    return sha256Stream(stream);
}

std::string sha256Text(const std::string &text)
{
    std::istringstream stream(text);
    return sha256Stream(stream);
}

bool diagnosticDigest(const std::string &value)
{
    return value.size() == 64 && std::all_of(value.begin(), value.end(), [](char character) {
               return (character >= '0' && character <= '9') ||
                      (character >= 'a' && character <= 'f');
           });
}

bool selfTestPathInside(fs::path candidate, fs::path parent)
{
    candidate = candidate.lexically_normal();
    parent = parent.lexically_normal();
    candidate.make_preferred();
    parent.make_preferred();
    return isSameOrInside(candidate, parent);
}

bool privateQaPath(const fs::path &path)
{
    if (!path.is_absolute() || path.wstring().size() > 32760)
    {
        return false;
    }
    for (const auto &component : path.relative_path())
    {
        auto text = component.wstring();
        if (text.empty() || text == L"." || text == L".." || text.back() == L'.' ||
            text.back() == L' ' || text.find(L':') != std::wstring::npos ||
            std::any_of(text.begin(), text.end(), [](wchar_t c) { return c < 32; }))
        {
            return false;
        }
    }
    fs::path allowed(L"E:\\Venom-Installer-QA");
    return selfTestPathInside(path, allowed) && !selfTestPathInside(allowed, path);
}

std::wstring directoryVolume(const fs::path &path)
{
    HANDLE directory = CreateFileW(
        path.c_str(), FILE_READ_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT, nullptr);
    selfTestRequire(directory != INVALID_HANDLE_VALUE, "Diagnostic directory unavailable");
    BY_HANDLE_FILE_INFORMATION information{};
    wchar_t resolved[32768];
    bool valid = GetFileInformationByHandle(directory, &information) &&
                 (information.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
                 !(information.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT);
    DWORD count = valid ? GetFinalPathNameByHandleW(directory, resolved, 32768,
                                                    FILE_NAME_NORMALIZED | VOLUME_NAME_GUID)
                        : 0;
    CloseHandle(directory);
    selfTestRequire(count > 0 && count < 32768, "Diagnostic directory resolution failed");
    std::wstring result(resolved, count);
    auto end = result.find(L"}\\");
    selfTestRequire(end != std::wstring::npos, "Diagnostic volume identity unavailable");
    return result.substr(0, end + 2);
}

void requirePrivateDirectory(const fs::path &path)
{
    selfTestRequire(privateQaPath(path) && fs::is_directory(path) && !hasReparsePoint(path),
                    "Diagnostics require an existing nonredirected private E QA directory");
    auto volume = directoryVolume(path);
    selfTestRequire(volume == directoryVolume(L"E:\\") && volume != directoryVolume(L"C:\\"),
                    "Diagnostics require physical E storage, distinct from C");
}

struct UiSelfTestScenario
{
    fs::path runRoot;
    fs::path sourceRoot;
    std::vector<std::wstring> values;
    std::string installerHash, backendHash, pythonHash, manifestHash, baselineReceiptHash,
        authorizedCommandHash;
    DWORD expectedExitCode = 0;
    unsigned timeoutSeconds = 0;
};

UiSelfTestScenario parseSelfTestScenario(const std::string &text)
{
    selfTestRequire(text.size() <= 65536 && text.find('\0') == std::string::npos,
                    "Scenario exceeds 64KiB or contains NUL");
    std::map<std::string, std::string> values;
    std::istringstream lines(text);
    std::string line;
    while (std::getline(lines, line))
    {
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }
        if (line.empty() || line.front() == '#')
        {
            continue;
        }
        selfTestRequire(
            std::none_of(line.begin(), line.end(), [](unsigned char c) { return c < 32; }),
            "Scenario control character refused");
        auto separator = line.find('=');
        selfTestRequire(separator != std::string::npos && separator > 0,
                        "Scenario requires key=value lines");
        selfTestRequire(
            values.emplace(line.substr(0, separator), line.substr(separator + 1)).second,
            "Duplicate scenario key");
    }
    std::set<std::string> keys = {"format",
                                  "mode",
                                  "authorization",
                                  "run-root",
                                  "package-root",
                                  "installer-sha256",
                                  "backend-sha256",
                                  "python-sha256",
                                  "package-manifest-sha256",
                                  "baseline-receipt-sha256",
                                  "authorized-command-sha256",
                                  "timeout-seconds",
                                  "expected-exit-code",
                                  "resume",
                                  "remember-inputs"};
    for (const auto &field : fields)
    {
        keys.insert("field." + strictUtf8(field.key));
    }
    // Preserve legacy pinned scenarios and their exact authorized command line.
    values.try_emplace("field.previous-output", "");
    selfTestRequire(values.size() == keys.size(), "Scenario field set is incomplete");
    for (const auto &[key, value] : values)
    {
        selfTestRequire(keys.contains(key), "Unknown scenario key");
    }
    selfTestRequire(values.at("format") == "1" && values.at("mode") == "resume-build" &&
                        values.at("authorization") == "I_AUTHORIZE_HASH_PINNED_RESUME" &&
                        values.at("resume") == "1" && values.at("remember-inputs") == "0",
                    "Explicit pinned resume authorization required; persistence disabled");
    auto number = [&](const char *key, unsigned maximum) {
        auto value = values.at(key);
        selfTestRequire(!value.empty() && value.size() <= 4 &&
                            value.find_first_not_of("0123456789") == std::string::npos,
                        "Scenario integer invalid");
        unsigned parsed = std::stoul(value);
        selfTestRequire(parsed <= maximum, "Scenario integer exceeds bound");
        return parsed;
    };
    UiSelfTestScenario scenario;
    scenario.timeoutSeconds = number("timeout-seconds", 1800);
    selfTestRequire(scenario.timeoutSeconds > 0, "Finite positive self-test deadline required");
    scenario.expectedExitCode = number("expected-exit-code", 255);
    scenario.runRoot = strictWide(values.at("run-root"));
    scenario.sourceRoot = strictWide(values.at("package-root"));
    selfTestRequire(privateQaPath(scenario.runRoot) && privateQaPath(scenario.sourceRoot),
                    "Scenario writable/source paths must be private E QA children");
    for (auto [key, destination] :
         {std::pair{"installer-sha256", &scenario.installerHash},
          std::pair{"backend-sha256", &scenario.backendHash},
          std::pair{"python-sha256", &scenario.pythonHash},
          std::pair{"package-manifest-sha256", &scenario.manifestHash},
          std::pair{"baseline-receipt-sha256", &scenario.baselineReceiptHash},
          std::pair{"authorized-command-sha256", &scenario.authorizedCommandHash}})
    {
        *destination = values.at(key);
        selfTestRequire(diagnosticDigest(*destination), "Scenario SHA256 invalid");
    }
    for (const auto &field : fields)
    {
        auto value = strictWide(values.at("field." + strictUtf8(field.key)));
        selfTestRequire(value.size() <= 32760, "Scenario form value exceeds control bound");
        scenario.values.push_back(std::move(value));
    }
    fs::path output(scenario.values[OutputField]), install(scenario.values[InstallField]);
    selfTestRequire(
        privateQaPath(output) && privateQaPath(install) && selfTestPathInside(install, output) &&
            !selfTestPathInside(output, install) && !selfTestPathInside(scenario.runRoot, output) &&
            !selfTestPathInside(output, scenario.runRoot),
        "Self-test requires a separate E QA output and explicit install child");
    return scenario;
}

std::string readScenario(const fs::path &path)
{
    selfTestRequire(fs::is_regular_file(path) && !hasReparsePoint(path) &&
                        fs::file_size(path) <= 65536,
                    "Scenario must be a bounded regular file");
    std::ifstream stream(path, std::ios::binary);
    selfTestRequire(bool(stream), "Cannot open scenario");
    std::string result(65537, '\0');
    stream.read(result.data(), static_cast<std::streamsize>(result.size()));
    result.resize(static_cast<size_t>(stream.gcount()));
    selfTestRequire(result.size() <= 65536 && stream.eof() && !stream.bad(),
                    "Scenario read failed");
    return result;
}

struct UiSelfTestState
{
    UiSelfTestScenario scenario;
    std::string scenarioHash, commandLine, failure;
    HANDLE receipt = INVALID_HANDLE_VALUE;
    uint64_t deadline = 0;
    bool reviewSeen = false, authorized = false, validationSeen = false,
         disabledDuringBuild = false, controlsContained = false, timedOut = false;
    int clientWidth = 0, clientHeight = 0;
    int exitCode = 2;
    ~UiSelfTestState()
    {
        if (receipt != INVALID_HANDLE_VALUE)
        {
            CloseHandle(receipt);
        }
    }
};
std::unique_ptr<UiSelfTestState> uiSelfTest;
std::atomic<bool> selfTestCancelRequested{false}, lastMonitorFailed{false}, lastChildExited{false},
    lastOwnedJobDrained{false}, lastJobAssigned{false};
std::atomic<DWORD> lastChildPid{0}, lastChildCode{STILL_ACTIVE};

void configureUiSelfTest(const fs::path &scenarioPath, const std::string &expectedHash,
                         const fs::path &executable)
{
    requirePrivateDirectory(scenarioPath.parent_path());
    selfTestRequire(scenarioPath.filename() == L"scenario.ini" && diagnosticDigest(expectedHash),
                    "Use a pinned scenario.ini in its own private run directory");
    auto text = readScenario(scenarioPath);
    selfTestRequire(sha256Text(text) == expectedHash, "Scenario hash mismatch");
    auto state = std::make_unique<UiSelfTestState>();
    state->scenario = parseSelfTestScenario(text);
    auto &scenario = state->scenario;
    selfTestRequire(fs::equivalent(scenario.runRoot, scenarioPath.parent_path()),
                    "Scenario/run directory mismatch");
    requirePrivateDirectory(scenario.sourceRoot);
    requirePrivateDirectory(scenario.values[OutputField]);
    selfTestRequire(!fs::exists(scenario.values[InstallField]) &&
                        !hasReparsePoint(scenario.values[InstallField]),
                    "Self-test install child must be fresh and nonredirected");
    selfTestRequire(
        sha256File(executable) == scenario.installerHash &&
            sha256File(scenario.sourceRoot / L"native/installer/Build-Install-Venom.py") ==
                scenario.backendHash &&
            sha256File(scenario.sourceRoot / L"PACKAGE-MANIFEST.json") == scenario.manifestHash &&
            sha256File(scenario.values[PythonField]) == scenario.pythonHash &&
            sha256File(fs::path(scenario.values[OutputField]) / L"build-install-receipt.json") ==
                scenario.baselineReceiptHash,
        "Installer/backend/package/Python/baseline receipt pin mismatch");
    selfTestRequire(!fs::exists(scenario.runRoot / L"temp") &&
                        !fs::exists(scenario.runRoot / L"ui-self-test-result.json"),
                    "Self-test temp/receipt already exists");
    selfTestRequire(CreateDirectoryW((scenario.runRoot / L"temp").c_str(), nullptr),
                    "Cannot create private self-test temp");
    state->receipt =
        CreateFileW((scenario.runRoot / L"ui-self-test-result.json").c_str(), GENERIC_WRITE,
                    FILE_SHARE_READ, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
    selfTestRequire(state->receipt != INVALID_HANDLE_VALUE, "Self-test receipt already exists");
    state->scenarioHash = expectedHash;
    packageRoot = scenario.sourceRoot;
    uiSelfTest = std::move(state);
}

std::vector<wchar_t> selfTestChildEnvironment()
{
    if (!uiSelfTest)
    {
        return {};
    }
    wchar_t *block = GetEnvironmentStringsW();
    selfTestRequire(block != nullptr, "Cannot read child environment");
    std::vector<std::wstring> entries;
    for (const wchar_t *entry = block; *entry; entry += wcslen(entry) + 1)
    {
        if (_wcsnicmp(entry, L"TEMP=", 5) && _wcsnicmp(entry, L"TMP=", 4))
        {
            entries.emplace_back(entry);
        }
    }
    FreeEnvironmentStringsW(block);
    auto temp = (uiSelfTest->scenario.runRoot / L"temp").wstring();
    entries.push_back(L"TEMP=" + temp);
    entries.push_back(L"TMP=" + temp);
    std::sort(entries.begin(), entries.end(),
              [](const auto &a, const auto &b) { return _wcsicmp(a.c_str(), b.c_str()) < 0; });
    std::vector<wchar_t> result;
    for (const auto &entry : entries)
    {
        result.insert(result.end(), entry.begin(), entry.end());
        result.push_back(0);
    }
    result.push_back(0);
    return result;
}

bool drainSelfTestJob(HANDLE job)
{
    JOBOBJECT_BASIC_ACCOUNTING_INFORMATION information{};
    if (!QueryInformationJobObject(job, JobObjectBasicAccountingInformation, &information,
                                   sizeof(information), nullptr))
    {
        return false;
    }
    if (information.ActiveProcesses && !TerminateJobObject(job, ERROR_CANCELLED))
    {
        return false;
    }
    auto deadline = GetTickCount64() + 5000;
    do
    {
        if (!QueryInformationJobObject(job, JobObjectBasicAccountingInformation, &information,
                                       sizeof(information), nullptr))
        {
            return false;
        }
        if (!information.ActiveProcesses)
        {
            return true;
        }
        Sleep(20);
    } while (GetTickCount64() < deadline);
    return false;
}

std::string jsonString(const std::string &value)
{
    std::string result = "\"";
    constexpr char hex[] = "0123456789abcdef";
    for (unsigned char character : value)
    {
        if (character == '\\' || character == '"')
        {
            result += '\\';
            result += character;
        }
        else if (character < 32)
        {
            result += "\\u00";
            result += hex[character >> 4];
            result += hex[character & 15];
        }
        else
        {
            result += character;
        }
    }
    return result + '"';
}

bool selfTestConfirmation(bool build, const std::vector<std::wstring> &values, bool resume)
{
    selfTestRequire(values == uiSelfTest->scenario.values && resume,
                    "Actual controls differ from authorized scenario");
    auto command = strictUtf8(makeCommandLine(makePipelineArguments(values, resume)));
    selfTestRequire(sha256Text(command) == uiSelfTest->scenario.authorizedCommandHash,
                    "Actual production command lacks exact scenario authorization");
    uiSelfTest->validationSeen = true;
    uiSelfTest->commandLine = command;
    if (!build)
    {
        uiSelfTest->reviewSeen = true;
        return false;
    }
    selfTestRequire(uiSelfTest->reviewSeen, "Production command review was not exercised");
    auto &scenario = uiSelfTest->scenario;
    selfTestRequire(
        sha256File(scenario.sourceRoot / L"native/installer/Build-Install-Venom.py") ==
                scenario.backendHash &&
            sha256File(scenario.sourceRoot / L"PACKAGE-MANIFEST.json") == scenario.manifestHash &&
            sha256File(scenario.values[PythonField]) == scenario.pythonHash &&
            sha256File(fs::path(scenario.values[OutputField]) / L"build-install-receipt.json") ==
                scenario.baselineReceiptHash &&
            !fs::exists(scenario.values[InstallField]),
        "Backend/Python/baseline/install target changed before authorization");
    uiSelfTest->authorized = true;
    return true;
}

bool requiredSelfTestControlsContained()
{
    RECT client{};
    selfTestRequire(GetClientRect(mainWindow, &client), "Cannot query own client rectangle");
    uiSelfTest->clientWidth = client.right;
    uiSelfTest->clientHeight = client.bottom;
    DWORD owner = 0;
    GetWindowThreadProcessId(mainWindow, &owner);
    selfTestRequire(owner == GetCurrentProcessId(), "Self-test HWND is not app-owned");
    for (HWND control : {reviewButton, buildButton, loadButton, openButton, rememberBox, resumeBox})
    {
        RECT rectangle{};
        if (!control || GetParent(control) != mainWindow || !GetWindowRect(control, &rectangle))
        {
            return false;
        }
        MapWindowPoints(nullptr, mainWindow, reinterpret_cast<POINT *>(&rectangle), 2);
        if (rectangle.left < 0 || rectangle.top < 0 || rectangle.right > client.right ||
            rectangle.bottom > client.bottom || rectangle.right <= rectangle.left ||
            rectangle.bottom <= rectangle.top)
        {
            return false;
        }
    }
    return true;
}

bool requiredSelfTestInputsEnabled(bool expected)
{
    for (HWND control : fieldInputs)
    {
        if (!control || bool(IsWindowEnabled(control)) != expected)
        {
            return false;
        }
    }
    for (HWND control : {reviewButton, buildButton, loadButton, rememberBox, resumeBox})
    {
        if (!control || bool(IsWindowEnabled(control)) != expected)
        {
            return false;
        }
    }
    return true;
}

void finishUiSelfTest(DWORD resultCode)
{
    KillTimer(mainWindow, SelfTestTimerId);
    auto &state = *uiSelfTest;
    auto &scenario = state.scenario;
    std::string pipelineHash, installHash;
    auto outputReceipt = fs::path(scenario.values[OutputField]) / L"build-install-receipt.json";
    auto installReceipt = fs::path(scenario.values[InstallField]) / L"venom-install.json";
    try
    {
        for (auto [path, hash] :
             {std::pair{outputReceipt, &pipelineHash}, std::pair{installReceipt, &installHash}})
        {
            if (fs::is_regular_file(path) && !hasReparsePoint(path))
            {
                *hash = sha256File(path);
            }
        }
        bool enabled = requiredSelfTestInputsEnabled(true);
        bool formUnchanged = collectInputs() == scenario.values;
        auto statusText = windowText(statusLabel);
        bool statusMatches = statusText.starts_with(
            scenario.expectedExitCode == 0 ? L"Build/install exited 0 with receipt present."
                                           : L"Build/install failed or completion unverified.");
        bool freshInstall = !installHash.empty() && !pipelineHash.empty() &&
                            pipelineHash != scenario.baselineReceiptHash;
        bool passed = state.failure.empty() && !state.timedOut && state.validationSeen &&
                      state.reviewSeen && state.authorized && state.controlsContained &&
                      state.disabledDuringBuild && enabled && formUnchanged && statusMatches &&
                      lastJobAssigned.load() && lastChildExited.load() &&
                      lastOwnedJobDrained.load() && !lastMonitorFailed.load() &&
                      lastChildPid.load() != 0 && resultCode == scenario.expectedExitCode &&
                      lastChildCode.load() == scenario.expectedExitCode &&
                      (scenario.expectedExitCode != 0 || freshInstall);
        state.exitCode = passed ? 0 : 2;
        std::ostringstream receipt;
        receipt << "{\n\"status\":"
                << jsonString(passed ? (scenario.expectedExitCode == 0
                                            ? "PASS_OWN_FORM_REAL_BACKEND_FLOW"
                                            : "PASS_EXPECTED_BACKEND_FAILURE_UI_FLOW")
                                     : "FAIL_OWN_FORM_REAL_BACKEND_FLOW")
                << ",\n\"scenarioSha256\":" << jsonString(state.scenarioHash)
                << ",\n\"failure\":" << jsonString(state.failure)
                << ",\n\"commandLine\":" << jsonString(state.commandLine)
                << ",\n\"commandSha256\":" << jsonString(sha256Text(state.commandLine))
                << ",\n\"backendSha256\":" << jsonString(scenario.backendHash)
                << ",\n\"ownPid\":" << GetCurrentProcessId()
                << ",\n\"childPid\":" << lastChildPid.load()
                << ",\n\"childExitCode\":" << lastChildCode.load()
                << ",\n\"completionCode\":" << resultCode
                << ",\n\"expectedExitCode\":" << scenario.expectedExitCode
                << ",\n\"productionValidationObserved\":"
                << (state.validationSeen ? "true" : "false")
                << ",\n\"productionReviewObserved\":" << (state.reviewSeen ? "true" : "false")
                << ",\n\"exactCommandConfirmationAuthorized\":"
                << (state.authorized ? "true" : "false")
                << ",\n\"childExited\":" << (lastChildExited.load() ? "true" : "false")
                << ",\n\"monitorFailed\":" << (lastMonitorFailed.load() ? "true" : "false")
                << ",\n\"timedOut\":" << (state.timedOut ? "true" : "false")
                << ",\n\"requiredActionControlRectanglesContained\":"
                << (state.controlsContained ? "true" : "false")
                << ",\n\"clientWidth\":" << state.clientWidth
                << ",\n\"clientHeight\":" << state.clientHeight
                << ",\n\"inputsDisabledDuringBuild\":"
                << (state.disabledDuringBuild ? "true" : "false")
                << ",\n\"inputsEnabledAfterCompletion\":" << (enabled ? "true" : "false")
                << ",\n\"formValuesUnchanged\":" << (formUnchanged ? "true" : "false")
                << ",\n\"statusLabelText\":" << jsonString(strictUtf8(statusText))
                << ",\n\"statusLabelMatchesExpectedExit\":" << (statusMatches ? "true" : "false")
                << ",\n\"ownedJobZeroActiveBeforeClose\":"
                << (lastOwnedJobDrained.load() ? "true" : "false")
                << ",\n\"baselineReceiptSha256\":" << jsonString(scenario.baselineReceiptHash)
                << ",\n\"pipelineReceiptSha256\":" << jsonString(pipelineHash)
                << ",\n\"installManifestSha256\":" << jsonString(installHash)
                << ",\n\"freshInstallReceiptEvidence\":" << (freshInstall ? "true" : "false")
                << ",\n\"rememberInputs\":false,\n\"resume\":true,\n\"formValues\":{";
        auto observed = collectInputs();
        for (int index = 0; index < FieldCount; ++index)
        {
            receipt << (index ? "," : "") << jsonString(strictUtf8(fields[index].key)) << ":"
                    << jsonString(strictUtf8(observed[index]));
        }
        receipt << "},\n\"gameLaunched\":false,\n\"controllerSettingsChanged\":false,"
                   "\n\"externalWindowAutomation\":false,\n\"visibleTextDpiVerified\":false,"
                   "\n\"receiptSemanticsIndependentlyParsed\":false\n}\n";
        auto bytes = receipt.str();
        selfTestRequire(bytes.size() <= 1024 * 1024, "Diagnostic receipt exceeds bound");
        DWORD written = 0;
        selfTestRequire(WriteFile(state.receipt, bytes.data(), static_cast<DWORD>(bytes.size()),
                                  &written, nullptr) &&
                            written == bytes.size() && FlushFileBuffers(state.receipt),
                        "Cannot write final self-test receipt");
    }
    catch (const std::exception &error)
    {
        state.exitCode = 2;
        auto message = std::string("Self-test receipt failure: ") + error.what() + "\n";
        DWORD written = 0;
        WriteFile(GetStdHandle(STD_ERROR_HANDLE), message.data(),
                  static_cast<DWORD>(message.size()), &written, nullptr);
    }
    CloseHandle(state.receipt);
    state.receipt = INVALID_HANDLE_VALUE;
    DestroyWindow(mainWindow);
}

void reviewAndStart(bool build);
void beginUiSelfTest()
{
    try
    {
        auto &state = *uiSelfTest;
        state.controlsContained = requiredSelfTestControlsContained();
        selfTestRequire(state.controlsContained, "Own action controls outside tested client size");
        for (int index = 0; index < FieldCount; ++index)
        {
            selfTestRequire(
                SetWindowTextW(fieldInputs[index], state.scenario.values[index].c_str()),
                "Cannot fill own form control");
        }
        SendMessageW(resumeBox, BM_SETCHECK, BST_CHECKED, 0);
        SendMessageW(rememberBox, BM_SETCHECK, BST_UNCHECKED, 0);
        selfTestRequire(collectInputs() == state.scenario.values,
                        "Own form value round trip failed");
        state.deadline = GetTickCount64() + state.scenario.timeoutSeconds * 1000ULL;
        selfTestRequire(SetTimer(mainWindow, SelfTestTimerId, 200, nullptr) != 0,
                        "Cannot create finite own-window self-test timer");
        reviewAndStart(false);
        selfTestRequire(state.failure.empty() && state.reviewSeen,
                        "Production command review failed");
        reviewAndStart(true);
        selfTestRequire(running && state.failure.empty(), "Production build handler did not start");
        state.disabledDuringBuild = requiredSelfTestInputsEnabled(false);
        selfTestRequire(state.disabledDuringBuild, "Production form not disabled during build");
    }
    catch (const std::exception &error)
    {
        uiSelfTest->failure = error.what();
        if (running)
        {
            selfTestCancelRequested.store(true);
        }
        else
        {
            finishUiSelfTest(ERROR_INVALID_DATA);
        }
    }
}

#ifdef VENOM_STATIC_TEST
unsigned selfTestScenarioRegressions()
{
    std::string valid = "format=1\nmode=resume-build\n"
                        "authorization=I_AUTHORIZE_HASH_PINNED_RESUME\n"
                        "run-root=E:/Venom-Installer-QA/ui-test/run\n"
                        "package-root=E:/Venom-Installer-QA/source-package\n"
                        "timeout-seconds=60\nexpected-exit-code=0\nresume=1\nremember-inputs=0\n";
    for (auto key :
         {"installer-sha256", "backend-sha256", "python-sha256", "package-manifest-sha256",
          "baseline-receipt-sha256", "authorized-command-sha256"})
    {
        valid += std::string(key) + "=" + std::string(64, 'a') + "\n";
    }
    for (const auto &field : fields)
    {
        std::string value = "E:/read-only/input";
        if (&field == &fields[OutputField])
            value = "E:/Venom-Installer-QA/pipeline";
        if (&field == &fields[InstallField])
            value = "E:/Venom-Installer-QA/pipeline/new-install";
        if (&field == &fields[JobsField])
            value = "2";
        if (&field == &fields[AptOverrideField] || &field == &fields[PreviousOutputField])
            value = "";
        valid += "field." + strictUtf8(field.key) + "=" + value + "\n";
    }
    auto scenario = parseSelfTestScenario(valid);
    selfTestRequire(scenario.values.size() == FieldCount && scenario.timeoutSeconds == 60,
                    "Valid self-test data did not parse");
    auto legacy = valid;
    auto optionalOffset = legacy.find("field.previous-output=\n");
    selfTestRequire(optionalOffset != std::string::npos, "Optional field fixture missing");
    legacy.erase(optionalOffset, std::string("field.previous-output=\n").size());
    selfTestRequire(parseSelfTestScenario(legacy).values == scenario.values,
                    "Omitted optional previous output changed legacy scenario");
    unsigned count = 2;
    auto reject = [&](const std::string &text) {
        bool rejected = false;
        try
        {
            parseSelfTestScenario(text);
        }
        catch (const std::exception &)
        {
            rejected = true;
        }
        selfTestRequire(rejected, "Invalid self-test data was accepted");
        ++count;
    };
    reject(valid + "format=1\n");
    reject(valid + "unknown=value\n");
    reject(std::string(65537, 'a'));
    reject(valid + std::string("\0", 1));
    auto replace = [&](const std::string &before, const std::string &after) {
        auto changed = valid;
        auto position = changed.find(before);
        selfTestRequire(position != std::string::npos, "Test mutation missing");
        changed.replace(position, before.size(), after);
        reject(changed);
    };
    replace("format=1", "format=2");
    replace("mode=resume-build", "mode=fake-success");
    replace("authorization=I_AUTHORIZE_HASH_PINNED_RESUME", "authorization=no");
    replace("resume=1", "resume=0");
    replace("remember-inputs=0", "remember-inputs=1");
    for (auto invalid : {"0", "1801", "-1", "99999", ""})
    {
        replace("timeout-seconds=60", std::string("timeout-seconds=") + invalid);
    }
    replace("expected-exit-code=0", "expected-exit-code=256");
    replace("run-root=E:/Venom-Installer-QA/ui-test/run", "run-root=C:/qa/run");
    replace("field.install-root=E:/Venom-Installer-QA/pipeline/new-install",
            "field.install-root=E:/Venom-Installer-QA/other");
    replace("package-root=E:/Venom-Installer-QA/source-package",
            "package-root=E:/Venom-Installer-QA/../outside");
    replace("installer-sha256=" + std::string(64, 'a'), "installer-sha256=bad");
    replace("field.python=E:/read-only/input", "field.python=" + std::string(1, char(255)));
    for (const fs::path &path :
         {fs::path(L"C:/qa/run"), fs::path(L"E:/Venom-Installer-QA"),
          fs::path(L"e:/venom-installer-qa"), fs::path(L"E:/Venom-Installer-QA/run/../outside"),
          fs::path(L"E:/Venom-Installer-QA/run:ads"), fs::path(L"E:/Venom-Installer-QA/run ")})
    {
        selfTestRequire(!privateQaPath(path), "Unsafe private diagnostic path accepted");
        ++count;
    }
    selfTestRequire(sha256Text("abc") ==
                        "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad",
                    "Diagnostic SHA256 regression");
    ++count;
    selfTestRequire(jsonString("a\"\\\n") == "\"a\\\"\\\\\\u000a\"",
                    "Diagnostic JSON escaping regression");
    ++count;
    return count;
}
#endif
