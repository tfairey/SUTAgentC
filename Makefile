target SUTAgent.out: doCommand.c CmdWorkerThread.c RunCmdThread.c SUTAgent.c
	gcc doCommand.c CmdWorkerThread.c RunCmdThread.c RedirOutputThread.c SUTAgent.c DataWorkerThread.c RunDataThread.c -lpthread
