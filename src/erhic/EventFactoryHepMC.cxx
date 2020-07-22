/**
   \file
   Implementation of class erhic::EventFactoryHepMC.

   \author    Chris Pinkenburg, Kolja Kauder
   \date      2020-07-07
   \copyright 2020 Brookhaven National Lab
*/

#include "eicsmear/erhic/EventFactoryHepMC.h"

#include <memory>
#include <stdexcept>
#include <string>

#include <TClass.h>
#include <TProcessID.h>

#include "eicsmear/erhic/BeamParticles.h"
#include "eicsmear/erhic/EventPythia.h"
#include "eicsmear/erhic/EventHepMC.h"
#include "eicsmear/erhic/EventMilou.h"
#include "eicsmear/erhic/EventDjangoh.h"
#include "eicsmear/erhic/EventDpmjet.h"
#include "eicsmear/erhic/EventRapgap.h"
#include "eicsmear/erhic/EventPepsi.h"
#include "eicsmear/erhic/EventGmcTrans.h"
#include "eicsmear/erhic/EventSimple.h"
#include "eicsmear/erhic/EventSartre.h"
#include "eicsmear/functions.h"  // For getFirstNonBlank()
#include "eicsmear/erhic/Kinematics.h"
#include "eicsmear/erhic/ParticleIdentifier.h"
#include "eicsmear/erhic/ParticleMC.h"

#include <TVector3.h>
#include <TParticlePDG.h>
#include <TLorentzVector.h>
#include <TDatabasePDG.h>
#include <TObjArray.h>
#include <TObjString.h>

#include <map>
#include <vector>
#include <algorithm> // for *min_element, *max_element

using std::cout;
using std::cerr;
using std::endl;
using std::map;
using std::vector;

namespace erhic {

  // Use this struct to automatically reset TProcessID object count.
  struct TProcessIdObjectCount {
    // Initialse object with current TProcessID object count.
    TProcessIdObjectCount() {
      count = TProcessID::GetObjectCount();
    }
    // Restore object count to the value at initialisation.
    // See example in $ROOTSYS/test/Event.cxx
    // To save space in the table keeping track of all referenced objects
    // we assume that our events do not address each other.
    ~TProcessIdObjectCount() {
      TProcessID::SetObjectCount(count);
    }
    int count;
  };

  template<>
  bool EventFromAsciiFactory<erhic::EventHepMC>::AtEndOfEvent() const {return false;}

  template<>
  void EventFromAsciiFactory<erhic::EventHepMC>::FindFirstEvent()  { /* Do nothing; the general template function skips 5 lines here.*/ }


  template<>
  bool EventFromAsciiFactory<erhic::EventHepMC>::AddParticle() {
    try {
      HepMC3::ReaderAsciiHepMC2 adapter2(*mInput);
      if (mEvent.get()) {
        HepMC3::GenEvent evt(HepMC3::Units::GEV,HepMC3::Units::MM);
	adapter2.read_event(evt);
	if ( adapter2.failed() )    return false;

	int particleindex;
	particleindex = 1;
	// Can't use GenParticle::children() because they don't have indices assigned yet
	// map each HepMC particle onto its corresponding particleindex
	std::map < HepMC3::GenParticlePtr, int > hepmcp_index;
	hepmcp_index.clear();

	// start with the beam plus gamma* and scattered lepton
	// Boring determination of the order:
	auto beams = evt.beams();
	assert ( beams.size() == 2 );
	auto hadron = beams.at(0); // or nucleon
	auto lepton = beams.at(1);
	// Find the lepton
	auto pdgl = TDatabasePDG::Instance()->GetParticle( lepton->pdg_id() );
	auto pdgh = TDatabasePDG::Instance()->GetParticle( hadron->pdg_id() );
	// Sigh. TParticlePDG uses C strings for particle type. I refuse to use strcmp
	// Could test individual pid's instead.
	bool b0islepton = (string(pdgl->ParticleClass()) == "Lepton");
	bool b1islepton = (string(pdgh->ParticleClass()) == "Lepton");
	if ( b0islepton == b1islepton ){
	  // exactly one of them should be a lepton.
	  throw std::runtime_error ("Exactly one beam should be a lepton - please contact the authors for ff or hh beams");
	}
	if ( !b0islepton ) {
	  std::swap (lepton, hadron);
	}
	// careful, don't try to use pdg[lh], b[01]islepton from here on out;
	// they don't get swapped along

	// now find the scattered e and the gamma
	// some processes (ff2ff) don't have a gamma in the event record
	HepMC3::GenParticlePtr scatteredlepton;
	HepMC3::GenParticlePtr photon;
	switch ( lepton->children().size() ){
	case 1: {
	    scatteredlepton = lepton->children().at(0);
	    auto photonmom = lepton->momentum() - scatteredlepton->momentum();
	    // Note that the m^2 = Q^2 seems low -- check
	    // 13 : incoming beam-inside-beam (e.g. gamma inside e) <-- not sure how correct for ff2ff, but we'll use it
	    photon = std::make_shared<HepMC3::GenParticle>( photonmom, 22, 13 );
	    photon->set_momentum(photonmom);
	    photon->set_pid( 22 );
	    // And add to the vertex (meaning the photon now has the lepton as a mother)
	    scatteredlepton->production_vertex()->add_particle_out(photon);
	    break;
	}
	case 2:
	  scatteredlepton = lepton->children().at(0);
	  photon = lepton->children().at(1);
	  if ( scatteredlepton->pid() != 22 && photon->pid() != 22 ){
	    cerr << "lepton child 1 pid = " << scatteredlepton->pid() << endl;
	    cerr << "lepton child 2 pid = " << photon->pid() << endl;
	    throw std::runtime_error ("Found two lepton daughters, none or both of them a photon.");
	  }
	  if ( photon->pid() != 22 ){
	    std::swap ( photon, scatteredlepton );
	  }
	  break;
	default:
	  cerr << "electron has " << lepton->children().size() << " daughters." << endl;
	  throw std::runtime_error ("Wrong number of lepton daughters (should be 1 or 2).");
	  break;
	}

	// Now add all four in the right order
	// Typical order is
	//   beams.SetBeamLepton(particles.at(0)->Get4Vector());
	//   beams.SetBeamHadron(particles.at(1)->Get4Vector());
	//   beams.SetBoson(particles.at(2)->Get4Vector());
	//   beams.SetScatteredLepton(particles.at(3)->Get4Vector());
	// While we don't _have_ to follow that, it makes sense to obey the convention
	// and not reinvent the wheel

	HandleHepmcParticle( lepton, hepmcp_index, particleindex, mEvent );
	HandleHepmcParticle( hadron, hepmcp_index, particleindex, mEvent );
	HandleHepmcParticle( photon, hepmcp_index, particleindex, mEvent );
	HandleHepmcParticle( scatteredlepton, hepmcp_index, particleindex, mEvent );

	// Now go over all vertices and handle the rest.
	// Note that by default this could double-count what we just did
	// Instead of trying to find out which vertex to skip, just use the lookup table
	// (inside HandleHepmcParticle) to avoid this.
	for (auto& v : evt.vertices() ){
	  for (auto& p : v->particles_out() ) {
	    HandleHepmcParticle( p, hepmcp_index, particleindex, mEvent );
	  }
	}

	// Now the map has built up full 1-1 correspondence between all hepmc particles
	// and the ParticleMC index.
	// So we can loop over the particles again, find their parents and offspring, and map accordingly.
	// We explicitly take advantage of particleid = Event entry # +1
	// If that changes, we'll need to maintain a second map
	// Note: the beam proton appears twice; that's consistent with the behavior of pythia6
	for (auto& v : evt.vertices() ){
	  for (auto& p : v->particles_out() ) {
	    // corresponding ParticleMC is at
	    int treeindex = hepmcp_index[p]-1;
	    assert ( treeindex >=0 ); // Not sure if that can happen. If it does, we could probably just continue;
	    auto track = mEvent->GetTrack(treeindex);

	    // collect all parents
	    vector<int> allparents;
	    for (auto& parent : p->parents() ) {
	      allparents.push_back( hepmcp_index[parent] );
	    }
	    // orig and orig1 aren't very intuitively named...
	    // For pythia6, orig is the default, and orig1 seems purely a placeholder
	    // trying to mimick that here.
	    if ( allparents.size() == 1){
	      track->SetParentIndex( allparents.at(0) );
	    }
	    if ( allparents.size() >= 2){
	      // smallest and highest are stored
	      track->SetParentIndex( *min_element(allparents.begin(), allparents.end() ) );
	      track->SetParent1Index( *max_element(allparents.begin(), allparents.end() ) );
	    }

	    // same for children
	    vector<int> allchildren;
	    for (auto& child : p->children() ) {
	      allchildren.push_back( hepmcp_index[child] );
	    }
	    if ( allchildren.size() == 1){
	      track->SetChild1Index( allchildren.at(0) );
	    }
	    if ( allchildren.size() >= 2){
	      // smallest and highest are stored
	      track->SetChild1Index( *min_element(allchildren.begin(), allchildren.end() ) );
	      track->SetChildNIndex( *max_element(allchildren.begin(), allchildren.end() ) );
	    }
	  }
	}
	// Update run-wise information.
	// Wasteful, since it's only written out for the final event, but
	// this class doesn't know when that happens
	TString xsec = ""; xsec += evt.cross_section()->xsec();
	TString xsecerr = ""; xsecerr += evt.cross_section()->xsec_err();
	TObjString* Xsec;
	TObjString* Xsecerr;

	// // not saved in the examples I used.
	// TString trials = ""; trials+= evt.cross_section()->get_attempted_events();
	// TString nevents = ""; nevents += evt.cross_section()->get_accepted_events();
	// TObjString* Trials;
	// TObjString* Nevents;

	if ( mObjectsToWriteAtTheEnd.size() == 0 ){
	  Xsec = new TObjString;
	  Xsecerr = new TObjString;
	  mObjectsToWriteAtTheEnd.push_back ( NamedObjects( "crossSection", Xsec) );
	  mObjectsToWriteAtTheEnd.push_back ( NamedObjects( "crossSectionError", Xsecerr) );
	  // Trials = new TObjString;
	  // Nevents = new TObjString;
	  // mObjectsToWriteAtTheEnd.push_back ( NamedObjects( "nTrials", Trials) );
	  // mObjectsToWriteAtTheEnd.push_back ( NamedObjects( "nEvents", Nevents) );
	}
	Xsec = (TObjString*) mObjectsToWriteAtTheEnd.at(0).second;
	Xsec->String() = xsec;
	Xsecerr = (TObjString*) mObjectsToWriteAtTheEnd.at(1).second;
	Xsecerr->String() = xsecerr;
	// Trials = (TObjString*) mObjectsToWriteAtTheEnd.at(2).second;
	// Trials->String() = trials;
	// Nevents = (TObjString*) mObjectsToWriteAtTheEnd.at(3).second;
	// Nevents->String() = nevents;
      }  // if

      auto finished = FinishEvent(); // 0 is success
      return (finished==0);
    }  // try
    catch(std::exception& error) {
      std::cerr << "Exception building particle: " << error.what() << std::endl;
      return false;
    }
  }

  template<>
  erhic::EventHepMC* EventFromAsciiFactory<erhic::EventHepMC>::Create()
  {
    TProcessIdObjectCount objectCount;
    mEvent.reset(new erhic::EventHepMC());
    if (!AddParticle()) {
      mEvent.reset(nullptr);
    }  // if

    return mEvent.release();
  }

  void HandleHepmcParticle( const HepMC3::GenParticlePtr& p, std::map < HepMC3::GenParticlePtr, int >& hepmcp_index, int& particleindex, std::unique_ptr<erhic::EventHepMC>& mEvent ){
    // do nothing if we already used this particle
    auto it = hepmcp_index.find(p);
    if ( it != hepmcp_index.end() ) return;

    // Create the particle
    ParticleMC particle;
    auto v = p->production_vertex();
    TVector3 vertex(v->position().x(),v->position().y(),v->position().z());
    particle.SetVertex(vertex);
    // takes care of  xv, yv, zv;

    TLorentzVector lovec(p->momentum().x(),p->momentum().y(),p->momentum().z(),p->momentum().e());
    particle.Set4Vector(lovec);
    // takes care of  E, px, py, pz, m, and
    // derived quantities: pt, p, rapidity, eta, theta, phi

    // fill up the missing parts
    particle.SetId( p->pid() );
    particle.SetStatus(p->status());

    // Index: Runs from 1 to N.
    particle.SetIndex ( particleindex );

    // remember this HepMC3::GenParticlePtr <-> index connection
    hepmcp_index[p] = particleindex;

    particleindex++;

    particle.SetEvent(mEvent.get());
    mEvent->AddLast(&particle);
  }

}  // namespace erhic

namespace {

  // Need this to generate the CINT code for each version
  erhic::EventFromAsciiFactory<erhic::EventHepMC> eh;
}  // namespace