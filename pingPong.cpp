#include "ace/config.h"
#include "ace/Task.h"
#include <map>
#include <string>
#include <iostream>

enum GameState {PING,PONG};

class PingPongTask : public ACE_Task_Base {
private:
	GameState* const ppState;
	ACE_Condition<ACE_Thread_Mutex> &cWaiter;
	ACE_Thread_Mutex &readerLock;
	const GameState &desiredState;
	const int lifetime;
	std::map<GameState,std::string>* stateNames;
public:
    virtual int svc(void);
	PingPongTask(GameState* ppState1,
				ACE_Condition<ACE_Thread_Mutex> &cWaiter1,
				ACE_Thread_Mutex &readerLock1,
				const GameState &desiredState1,
				const int lifetime1,
				std::map<GameState,std::string>* stateNames1) :
		ppState(ppState1), cWaiter(cWaiter1), readerLock(readerLock1),
		desiredState(desiredState1), lifetime(lifetime1), stateNames(stateNames1)
	{}
};

int PingPongTask::svc(void) {
	//ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("(%t) Handler Thread running\n")));
	for(int i = 0; i < this->lifetime; i++) {
		this->readerLock.acquire();
		while(*ppState != this->desiredState) {
			this->cWaiter.wait();
		}
		this->readerLock.release();
		//print desired message from map
		std::cout << stateNames->at(this->desiredState) << std::endl;
		//switch states
		*ppState = this->desiredState == GameState::PING ? GameState::PONG : GameState::PING;
		this->cWaiter.broadcast(); //let everyone else know that there's a new value
	}
	return 0;
}

int ACE_TMAIN(int argc, ACE_TCHAR *argv[]) {
	ACE_Thread_Mutex pingMutex;
	ACE_Condition<ACE_Thread_Mutex> pingWait(pingMutex);

	GameState ppState = GameState::PING;
	GameState * const statePtr = &ppState; //make the pointing location constant

	std::map<GameState,std::string> GameStrings; //prepare strings
	GameStrings[GameState::PING] = "Ping!";
	GameStrings[GameState::PONG] = "Pong!";
	
	PingPongTask ping = PingPongTask(statePtr,pingWait,pingMutex,GameState::PING,3,&GameStrings);
	PingPongTask pong = PingPongTask(statePtr,pingWait,pingMutex,GameState::PONG,3,&GameStrings);

	ping.activate();
	pong.activate();

	//ping.wait();
	pong.wait(); //only one thread needs to wait
	std::cout << "Done!" << std::endl;
	return 0;
}