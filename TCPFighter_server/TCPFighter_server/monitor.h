#ifndef __MONITOR__

enum MONITORING {START=0,END};

class Monitor
{
public:
	Monitor();
	void MonitorInit();
	void Run();
	void MonitorNetwork(MONITORING timing);
	void MonitorGameLogic(MONITORING timing);

	void SetNetwork() { bNetwork = !bNetwork; }
	bool GetNetwork() { return bNetwork; }
private:
	void ShowNetwork();
private:

	int loop_cnt;// = 0;
	int frame_cnt;// = 0;

	int logicTick;// = 0;
	unsigned int logicTickMin;// = -1;
	unsigned int logicTickMax;// = 0;

	int networkTick;// = 0;
	unsigned int networkTickMin;// = -1;
	unsigned int networkTickMax;// = 0;

	int checkTime;
	int min_check;
	int sec_cnt;

	bool bNetwork;
};

extern Monitor monitorUnit;

void ServerControl();

//void ShowNetwork();
//void ShowPacketCount();
//
////void Monitor();
//void MonitorInit();

#endif // !__MONITOR__
