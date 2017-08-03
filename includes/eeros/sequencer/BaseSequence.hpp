#ifndef ORG_EEROS_SEQUENCER_BASESEQUENCE_HPP_
#define ORG_EEROS_SEQUENCER_BASESEQUENCE_HPP_

#include <eeros/logger/Logger.hpp>
#include <eeros/sequencer/ConditionTimeout.hpp>
#include <eeros/sequencer/Monitor.hpp>
#include <vector>


namespace eeros {
	namespace sequencer {
		
		class Sequencer;
		
		enum class SequenceState {
			idle,		// upon creation
			running,	// active and running
			paused,		// not used
			aborting,	// to be stopped, due to restarting of caller sequence
			aborted,	// stopped, due to restarting of caller sequence
			terminated,	// precondition failed, exitcondition successful
			restarting,	// repeat 
		};

		class BaseSequence {			
		public:
			BaseSequence(Sequencer& seq, BaseSequence* caller);
			virtual ~BaseSequence();
			
		// 	virtual int operator()(std::string args) = 0;	//has to be implemented in derived class
			
// 			void pauseSequence();	not yet implemented
// 			void resumeSequence();	not yet implemented
			
			// This functions can be overwritten by a userdefinde, derived class
			// ////////////////////////////////////////////////////////////////////////
			virtual int start() = 0;
			virtual bool checkPreCondition();
			virtual bool checkExitCondition();
			virtual bool isStep() const;
			
			void setName(std::string name);
			std::string getName() const;
			void setId(int id);
			int getId() const;		// steps allways have id=-99
			void setBlocking();		// standard run mode
			void setNonBlocking();
			bool isBlocking() const;
			
			BaseSequence* getCallerSequence();
			std::vector< BaseSequence* > getCallerStack() const;
			
// 			void setRunningState(runningStateEnum runningState);
// 			runningStateEnum getRunningState() const;
			void restartSequence();
			
			void setPollingTime(int timeInMilliseconds);
			
			// Monitors
			// ////////////////////////////////////////////////////////////////////////
			std::vector< Monitor* > monitors;
			void addMonitor( Monitor* monitor);
			std::vector< Monitor* > getMonitors() const;
			
			// Timeout
			// ////////////////////////////////////////////////////////////////////////
			void setTimeoutTime(double timeoutInSec);		//in seconds. For this sequence
			void resetTimeout();
			void setTimeoutBehavior(SequenceProp behavior);	
			void setTimeoutExceptionSequence(BaseSequence* sequence);
			
			
		protected:
			virtual int action();		// handles different checks like preconditions
			
			Sequencer& seq;				//reference to singleton Sequencer
			bool isMainSequence = false;
			
			eeros::logger::Logger log;
			std::string name;
// 			std::string name = "Steps do not have a name";	//constructor of "Sequence" overwrites name
			
			SequenceState runningState;	
			bool blocking;			//standard run mode
			
			BaseSequence* callerSequence;
			bool exceptionIsActive = false;
			
		private:
// 			virtual int action() = 0;	//overwritten by "Sequence" and "Step"
	
			void checkMonitorsOfBlockedCallers();
			void checkMonitorsOfThisSequence();
			void checkMonitor(Monitor* monitor);
			
			void setActiveException(Monitor* activeMonitor);
			void clearActiveException();
			void checkActiveException();
			
			int id;
			bool sequenceIsRestarting = false;
			
			std::vector< BaseSequence* > callerStack;			//vector with all caller sequences. Top element is latest caller
			std::vector< BaseSequence* > callerStackBlocking;	//vector with all sequences, which are blocked by this sequence. Element[0] is the oldest blocked caller
			bool callerStackBlockingCreated = false;
			
			Monitor monitorTimeout;
			ConditionTimeout conditionTimeout;
			
			int restartCounter = 0;
			int pollingTime;		//in milliseconds for checkExitCondition monitors)
			
			Monitor* activeException;
			
			
		};
	};	//namespace sequencer
}; // namespace eeros

#endif // ORG_EEROS_SEQUENCER_BASESEQUENCE_HPP_