#ifndef ORG_EEROS_CONTROL_D_HPP_
#define ORG_EEROS_CONTROL_D_HPP_

#include <eeros/control/Block1i1o.hpp>

namespace eeros {
	namespace control {

		template < typename T = double >
		class D: public Block1i1o<T> {
			
		public:
			D() : first(true) {
				this->out.getSignal().clear();
			}
			
			virtual void run() {
				if(first) {  // first run, no previous value available -> set output to zero
					prev = this->in.getSignal();
					this->out.getSignal().setValue(0.0);
					this->out.getSignal().setTimestamp(this->in.getSignal().getTimestamp());
					first = false;
				}
				else {
					double tin = this->in.getSignal().getTimestamp() / 1000000000.0;
					double tprev = this->prev.getTimestamp() / 1000000000.0;
					T valin = this->in.getSignal().getValue();
					T valprev = this->prev.getValue();
					
					T valOut;
					if ((tin - tprev) == 0)		valOut = valprev;
					else 						valOut = (valin - valprev) / (tin - tprev);
					
					this->out.getSignal().setValue(valOut);
					this->out.getSignal().setTimestamp((this->in.getSignal().getTimestamp() + this->prev.getTimestamp()) / 2);
					
					prev = this->in.getSignal();
				}
			}
			
		protected:
			Signal<T> prev;
			bool first;
		};
	};
};
#endif /* ORG_EEROS_CONTROL_D_HPP_ */
