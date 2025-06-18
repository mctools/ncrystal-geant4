
// Small self-contained example application of G4NCrystal, where the NCrystal
// physics is injected via the Geant4 Biasing framework, supporting
// multi-threads Geant4 runs.


// We simulate an idealistic neutron scattering experiment, where a very small
// spherical Aluminium sample is surrounded by a much larger spherical detector
// where hits are recorded and, from their position, the scattering angle is
// inferred and printed. Due to the usage of G4NCrystal, the scattering in the
// sample will correctly be dominated by diffraction in the polycrystalline
// Aluminium, and the resulting spectrum will be dominated by scatterings at
// certain angles corresponding to the Bragg edges of Aluminium.

#include "G4NCrystal/G4NCrystal.hh"

#include "G4SystemOfUnits.hh"
#include "G4RunManagerFactory.hh"
#include "G4VUserDetectorConstruction.hh"
#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4VUserActionInitialization.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4PhysListFactory.hh"
#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4Sphere.hh"
#include "G4PVPlacement.hh"
#include "G4VSensitiveDetector.hh"
#include "G4SDManager.hh"
#include "G4StepPoint.hh"
#include "G4ios.hh"
#include "G4GenericBiasingPhysics.hh"


namespace {

  namespace Options {
    // Some global parameters for the test.
    constexpr auto sampleMaterial = "Al_sg225.ncmat;temp=350K"; // NCrystal cfg-string
    constexpr unsigned long nEvents = 100000;//sample size
    constexpr auto sampleRadius = 0.2*CLHEP::cm;
  }

  // Very simplistic but MT-safe recording of tallies:
  struct SDData {
    std::mutex mtx;
    unsigned long nScattered = 0;
    unsigned long nScattered162 = 0;
    unsigned long nScattered118 = 0;
    unsigned long nUnscattered = 0;
  };

  SDData& getRecordDB()
  {
    static SDData data;
    return data;
  }

}

class MySD : public G4VSensitiveDetector {
  // Sensitive detector for monitoring neutron hits in the spherical detector
public:
  MySD() : G4VSensitiveDetector( "MySD" ) {}
  G4bool ProcessHits( G4Step* step, G4TouchableHistory* ) final {
    if ( step->GetPreStepPoint()->GetStepStatus() != fGeomBoundary )    return true;  // only record at entry
    if ( step->GetTrack()->GetDynamicParticle()->GetPDGcode() != 2112 ) return true;  // must be neutron
    G4ThreeVector pos = step->GetPreStepPoint()->GetPosition();
    double r = sqrt( pos.x()*pos.x() + pos.y()*pos.y() );
    auto& record = getRecordDB();
    std::lock_guard<std::mutex> guard(record.mtx);
    if ( pos.z() > 0.0  &&  r < 0.001*mm ) {
      ++record.nUnscattered;
    } else {
      const double scat_deg = ( atan2( r, pos.z() ) / CLHEP::degree );
      if ( record.nScattered < 100 )
        G4cout << "Hit detected at theta = "<< scat_deg << " deg" << G4endl;
      ++record.nScattered;
      if ( fabs( scat_deg - 162.0 ) < 1.0 )
        ++record.nScattered162;
      if ( fabs( scat_deg - 118.0 ) < 1.0 )
        ++record.nScattered118;
    }
    return true;
  }
};

class MyGeo final : public G4VUserDetectorConstruction {
  // Constructs an r=Options::sampleRadius spherical sample inside an r=100*cm
  // spherical vacuum inside a 1*mm thick spherical counting volume, inside a
  // 110*cm world box. The sample is small enough for multiple neutron scattering
  // events to be negligible and the detector is far enough from the sample to
  // make sample size effects on the angular measurement equally negligible.
public:
  G4VPhysicalVolume* Construct() override {
    G4Material* mat_vacuum = G4NistManager::Instance()->FindOrBuildMaterial( "G4_Galactic", true );
    G4Material * mat_sample = G4NCrystal::createMaterial(Options::sampleMaterial);
    G4LogicalVolume* world_log = new G4LogicalVolume( new G4Box( "world", 110.0*cm, 110.0*cm, 110.0*cm ), mat_vacuum, "world", 0, 0, 0 );
    G4PVPlacement* world_phys = new G4PVPlacement( 0, G4ThreeVector(), world_log, "world", 0, false, 0 );
    G4LogicalVolume* det_log =
      new G4LogicalVolume( new G4Sphere( "detector", 0, 100.1*cm, 0, CLHEP::twopi, 0, CLHEP::pi ), mat_vacuum, "detector", 0, 0, 0 );
    new G4PVPlacement( 0, G4ThreeVector(), det_log, "detector", world_log, false, 0 );
    G4LogicalVolume* vacuum_log =
      new G4LogicalVolume( new G4Sphere( "vacuum", 0, 100.0*cm, 0, CLHEP::twopi, 0, CLHEP::pi ), mat_vacuum, "vacuum", 0, 0, 0 );
    new G4PVPlacement( 0, G4ThreeVector(), vacuum_log, "vacuum", det_log, false, 0 );
    fSampleLog = new G4LogicalVolume( new G4Sphere( "sample", 0, Options::sampleRadius, 0, CLHEP::twopi, 0, CLHEP::pi ),
                                      mat_sample, "sample", 0, 0, 0 );
    new G4PVPlacement( 0, G4ThreeVector(), fSampleLog, "sample", vacuum_log, false, 0 );
    return world_phys;
  }
  void ConstructSDandField() override {
    MySD* sd = new MySD;
    G4SDManager::GetSDMpointer()->AddNewDetector( sd );
    SetSensitiveDetector( "detector", sd ); // first parameter is name of
                                            // det_log logical volume above

    // Instantiate the biasing operator and attach to the volumes with NCrystal
    // materials (TODO: see if we can possibly automate this to simply bias any
    // volume where the material has an NCrystal scatter property:

    auto bias = new G4NCrystal::NCrystalBiasingOperator;
    bias->AttachTo( fSampleLog );
  }
private:
  G4LogicalVolume* fSampleLog = nullptr;
};

class MyGun : public G4VUserPrimaryGeneratorAction {
  // Monochromatic source of neutrons, hitting the sample with initial direction (0, 0, 1)
public:
  MyGun( double neutron_wavelength ) : m_particleGun( new G4ParticleGun(1) ) {
    m_particleGun->SetParticleDefinition( G4ParticleTable::GetParticleTable()->FindParticle( "neutron" ) );
    m_particleGun->SetParticleEnergy( neutronWavelengthToEKin( neutron_wavelength ) );
    m_particleGun->SetParticlePosition( G4ThreeVector( 0.0, 0.0, -1.0*cm ) );
    m_particleGun->SetParticleMomentumDirection( G4ThreeVector(0.0, 0.0, 1.0 ) );
  }
  void GeneratePrimaries( G4Event* evt ) final { m_particleGun->GeneratePrimaryVertex( evt ); }
  double neutronWavelengthToEKin( double wl ) {
    return 0.5 * CLHEP::h_Planck * CLHEP::h_Planck * CLHEP::c_squared / (wl*wl*CLHEP::neutron_mass_c2);
  }
private:
  std::unique_ptr< G4ParticleGun > m_particleGun;
};

class MyActions final : public G4VUserActionInitialization {
  // Actions, for registering primary generator
public:
  MyActions( double neutron_wavelength ) : m_neutron_wavelength( neutron_wavelength ) {}
  void Build() const override { SetUserAction( new MyGun( m_neutron_wavelength ) ); }
  double m_neutron_wavelength;
};

int main( int, char** ) {
  // Run-manager:

  // We simply use the runmanager factory and use the env var
  // G4FORCE_RUN_MANAGER_TYPE if we want to switch MT/Serial/Tasking mode (and
  // G4FORCENUMBEROFTHREADS if needed):

  auto* runManager = G4RunManagerFactory::CreateRunManager( G4RunManagerType::Default );
  CLHEP::HepRandom::setTheSeed( 123 );  // Set seed
  runManager->SetUserInitialization( new MyGeo );  // Setup geometry

  // Setup HP physics-list:
  G4VModularPhysicsList* physicsList = G4PhysListFactory().GetReferencePhysList( "QGSP_BIC_HP" );

  //Ensure biasing physics is enabled for neutrons:
  G4GenericBiasingPhysics* biasingPhysics = new G4GenericBiasingPhysics;
  biasingPhysics->Bias( "neutron" );
  physicsList->RegisterPhysics( biasingPhysics );
  runManager->SetUserInitialization( physicsList );

  // Setup monochromatic source of 4.0 A neutrons. Note that at 4.0 A
  // more than 90% of scattering events in polycrystalline aluminium are coherent
  // with peaks only at theta = 118 deg and theta = 162 deg:

  const double neutron_wavelength = 4.0*angstrom;
  runManager->SetUserInitialization( new MyActions( neutron_wavelength ) );
  runManager->Initialize();  // Initialize g4 run manager
  runManager->BeamOn( Options::nEvents );  // Perform simulations
  delete runManager;

  // Produce some output
  auto& record = getRecordDB();
  std::lock_guard<std::mutex> guard(record.mtx);

  G4cout << "#Events  :" << Options::nEvents << G4endl;
  const unsigned long nHits = record.nScattered + record.nUnscattered;
  G4cout << "#SD hits :" << nHits << G4endl;
  const double frac_scat = double( record.nScattered ) / nHits;
  const double frac_absorbed = double( Options::nEvents - nHits ) / nHits;
  G4cout << "#Fraction scattered: " << frac_scat * 100.0 << "%" << G4endl;
  G4cout << "#Fraction absorbed: " << frac_absorbed * 100.0 << "%" << G4endl;
  const double frac_118 = record.nScattered118 / double(record.nScattered);
  const double frac_162 = record.nScattered162 / double(record.nScattered);
  G4cout << "#Fraction of these scattered around 118deg: "
         << frac_118 * 100.0 << "%" <<  G4endl;
  G4cout << "#Fraction of these scattered around 162deg: "
         << frac_162 * 100.0 << "%" <<  G4endl;

  //We expect around 80% of events scattered on the two specific planes, so we
  //can use that to test if NCrystal was indeed controlling the physics here:
  if ( frac_118 + frac_162 < 0.5 ) {
    G4cerr << "NCrystal process seems to not have been activated!" << G4endl;
    return 1;
  }

  return 0;
}
