#ifndef ORG_EEROS_HALTEST1_HPP_
#define ORG_EEROS_HALTEST1_HPP_

#include <eeros/control/PeripheralOutput.hpp>
#include <eeros/control/PeripheralInput.hpp>
#include <eeros/control/Constant.hpp>
#include <eeros/control/TimeDomain.hpp>
#include <eeros/safety/SafetySystem.hpp>
#include <eeros/hal/HAL.hpp>
#include <eeros/core/Executor.hpp>
#include <eeros/sequencer/Sequence.hpp>
#include <eeros/sequencer/Sequencer.hpp>
#include <eeros/sequencer/Step.hpp>

using namespace eeros::safety;
using namespace eeros::control;
using namespace eeros::sequencer;
using namespace eeros::hal;

const double dt = 0.001;

class MyControlSystem {
public:
	MyControlSystem(double ts): digIn0("dIn0"), digIn1("dIn1"), digOut0("dOut0"), digOut1("dOut1"), 
					anIn0("aIn0"), anIn2("aIn2"), anOut0("aOut0"), anOut2("anOut2"),
					c0(true), c1(-2.0), c2(3.0), timedomain("Main time domain", ts, true) {
		digIn0.getOut().getSignal().setName("digital input 0");
		digIn1.getOut().getSignal().setName("digital input 1");
		anIn0.getOut().getSignal().setName("analog input 0");
		anIn2.getOut().getSignal().setName("analog input 2");
		anOut0.getIn().connect(c1.getOut());
		anOut2.getIn().connect(c2.getOut());
		digOut0.getIn().connect(c0.getOut());
		digOut1.getIn().connect(digIn0.getOut());
		timedomain.addBlock(c0);
		timedomain.addBlock(digOut0);
		timedomain.addBlock(digIn0);
		timedomain.addBlock(digOut1);
		timedomain.addBlock(digIn1);
		timedomain.addBlock(c1);
		timedomain.addBlock(c2);
		timedomain.addBlock(anOut0);
		timedomain.addBlock(anIn0);
		timedomain.addBlock(anOut2);
		timedomain.addBlock(anIn2);
		eeros::Executor::instance().add(timedomain);
	}
	
	Constant<bool> c0;
	Constant<> c1, c2;
	PeripheralOutput<bool> digOut0, digOut1;
	PeripheralInput<bool> digIn0, digIn1;
	PeripheralOutput<double> anOut0, anOut2;
	PeripheralInput<double> anIn0;
	PeripheralInput<double> anIn2;
	TimeDomain timedomain;
};

class MySafetyProperties : public SafetyProperties {
public:
	MySafetyProperties() : slSingle("single level") {
		addLevel(slSingle);
		setEntryLevel(slSingle);
	}
	virtual ~MySafetyProperties() { }
	SafetyLevel slSingle;
};

class WaitTime : public Step {
public:
	WaitTime(std::string name, Sequencer& seq, BaseSequence* caller) : Step(name, seq, caller) { }
	int operator() (double waitingTime) {this->waitingTime = waitingTime; return start();}
	int action() {time = std::chrono::steady_clock::now();}
	bool checkExitCondition() {return ((std::chrono::duration<double>)(std::chrono::steady_clock::now() - time)).count() > waitingTime;}
	std::chrono::time_point<std::chrono::steady_clock> time;
	double waitingTime;
};

class StepDigOut : public Step {
public:
	StepDigOut(std::string name, Sequencer& sequencer, BaseSequence* caller, MyControlSystem& cs) : Step(name, sequencer, caller), cs(cs) { }
	int operator() (bool val) {state = val; return start();}
	int action() {cs.c0.setValue(state);}
	bool state;
	MyControlSystem& cs;
};

class StepAnalogOut : public Step {
public:
	StepAnalogOut(std::string name, Sequencer& sequencer, BaseSequence* caller, MyControlSystem& cs) : Step(name, sequencer, caller), cs(cs) { }
	int operator() (bool val) {state = val; return start();}
	int action() {
		if (state) {
			cs.c1.setValue(6.0);
			cs.c2.setValue(-3.2);
		} else {
			cs.c1.setValue(-2.0);
			cs.c2.setValue(7.6);
		}
	}
	bool state;
	MyControlSystem& cs;
};

class SeqDigital : public Sequence {
public:
	SeqDigital(std::string name, Sequencer& sequencer, BaseSequence* caller, MyControlSystem& cs) : 
		Sequence(name, sequencer, caller, false), stepDigOut("step dig out", seq, this, cs), waitTime("waiting time digital", seq, this) { }
	int action() {
		bool toggle;
		while (Sequencer::running) {
			stepDigOut(toggle);
			waitTime(5);
			toggle = !toggle;
		}
	}
private:
	StepDigOut stepDigOut;
	WaitTime waitTime;
};

class SeqAnalog : public Sequence {
public:
	SeqAnalog(std::string name, Sequencer& sequencer, BaseSequence* caller, MyControlSystem& cs) : 
		Sequence(name, sequencer, caller, false), stepAnalogOut("step analog out", seq, this, cs), waitTime("waiting time analog", seq, this) { }
	int action() {
		bool toggle;
		while (Sequencer::running) {
			stepAnalogOut(toggle);
			waitTime(10);
			toggle = !toggle;
		}
	}
private:
	StepAnalogOut stepAnalogOut;
	WaitTime waitTime;
};

class MyMainSequence : public Sequence {
public:
	MyMainSequence(Sequencer& seq, MyControlSystem& cs) : Sequence("main", seq), cs(cs), seqDigital("seq dig out", seq, this, cs), seqAnalog("seq analog out", seq, this, cs) { }
	
	int action() {
		seqDigital();
		seqAnalog();
		seqDigital.wait();
		seqAnalog.wait();
	}
private:
	MyControlSystem& cs;
	SeqDigital seqDigital;
	SeqAnalog seqAnalog;
};

#endif // ORG_EEROS_HALTEST1_HPP_
