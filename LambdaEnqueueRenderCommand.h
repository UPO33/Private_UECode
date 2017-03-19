#include "RenderingThread.h"


template<typename TLambda> void EnqueueRenderCommand(const TLambda& cmd)
{
	struct Task : public FRenderCommand
	{
		TLambda mCmd;
		Task(const TLambda& _cmd) : mCmd(_cmd) {}

		void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent)
		{
			FRHICommandListImmediate& RHICmdList = GetImmediateCommandList_ForRenderCommand();
			mCmd();
		}
		TStatId GetStatId() const
		{
			RETURN_QUICK_DECLARE_CYCLE_STAT(TypeName, STATGROUP_RenderThreadCommands);
		}
	};

	LogRenderCommand(TypeName);
	if (ShouldExecuteOnRenderThread())
	{
		CheckNotBlockedOnRenderThread();
		check(ENamedThreads::GameThread != ENamedThreads::RenderThread);
		TGraphTask<Task>::CreateTask().ConstructAndDispatchWhenReady(cmd);
	}
	else
	{
		Task TempCommand(cmd);
		FScopeCycleCounter EURCMacro_Scope(TempCommand.GetStatId());
		TempCommand.DoTask(ENamedThreads::GameThread, FGraphEventRef());
	}
}