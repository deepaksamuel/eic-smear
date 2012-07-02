/**
 Pythia6EventFactory.h
 
 \file
 Declaration of class Pythia6EventFactory.
 
 \author Thomas Burton 
 \date 5/3/12
 \copyright 2012 BNL. All rights reserved.
 */

#ifndef _EICSMEAR_HADRONIC_PYTHIA6EVENTFACTORY_H_
#define _EICSMEAR_HADRONIC_PYTHIA6EVENTFACTORY_H_

#include <memory>
#include <string>

#include <Rtypes.h> // For ClassDef macro

#include <eicsmear/hadronic/EventPythia.h>
#include <eicsmear/erhic/EventFactory.h>
#include <eicsmear/erhic/EventMCFilterABC.h>

class TBranch;

namespace erhic {
   namespace hadronic {
      /**
       A realisation of erhic::VirtualEventFactory generating
       objects of type hadronic::EventPythiaPP.
       */
      class Pythia6EventFactory : public erhic::VirtualEventFactory {
      public:
         virtual ~Pythia6EventFactory();
         Pythia6EventFactory(EventMCFilterABC*);
         virtual EventPythiaPP* Create();
         virtual std::string EventName() const;
         virtual TBranch* Branch(TTree&, const std::string&);
         virtual void Fill(TBranch&);
      protected:
         virtual EventPythiaPP* BuildEvent();
         std::auto_ptr<erhic::EventMCFilterABC> mFilter;
         EventPythiaPP* mEvent;
         ClassDef(erhic::hadronic::Pythia6EventFactory, 1)
      };
   } // namespace hadronic
} // namespace erhic

#endif