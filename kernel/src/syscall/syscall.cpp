#include <syscall/syscall.hpp>
#include <process/process.hpp>
#include <panic.hpp>

SYSCALL(SYSCALL_EXIT, syscall, exitCode)
{
    auto currentProcess = Process::Current();
    if (currentProcess->GetProcessId() == 0)
    {
        kPanic("Idle thread called exit!");
    }
    currentProcess->State = ProcessState::Exited;
    currentProcess->ExitCode = exitCode;
    auto processList = *Process::GetProcessList();
    for (auto item : processList)
    {
        auto process = item.Value;
        if (process->GetProcessId() == 0)
        {
            process->Activate();
            break;
        }
    }
}