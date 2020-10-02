#include "TRACER_Demo.h"

#include "DCEL/Grid_Tools.h"
#include "TRACE/tracer.h"
#include <QDEBUG>
#include <QThread>
#include <QMutex>

enum States {
  Reset = 0, // should ALWAYS be 0
  Terrain, //visualization step only
  Nodes,
  Clean,
  Structure,
  Roads,
  Blocks, //not yet implemented
  Building, //not yet implemented
  Finished //marks completion, should ALWAYS be last, should NEVER be selectable
};

States& operator++(States& d)
{
  return d = (d == States::Finished) ? States::Finished : static_cast<States>(static_cast<int>(d) + 1);
}

const char * state_names[] = {
  "Reset",
  "Generate Terrain",
  "Generate Node Graph",
  "Clean Node Graph",
  "Generate Road Structure",
  "Generate Road Geometry",
  "Generate Block Geometry",
  "Generate Building Regions",
  "Finished"
};

//#define DEBUG_INTERSECTIONS

class TRACER_Demo::Data : public QThread {
	Q_OBJECT

public:
	virtual ~Data() override {};
	Data(QWidget* parent)
		: QThread(parent) {};

  void doReset(){
    const grd r = 400;
    if (current_config)
      delete current_config;

    srand(seed);

    current_config = new Network(400);

    for (auto i = 0; i < 6; i++) {
      current_config->terrain.AddEffect(new Grid_Effect(Pgrd(pRandom(-400, 400), pRandom(-400, 400)), randomQuarterUnitVector() * pRandom(1, 15)));
    }

    /*
      terrain.AddEffect(new Radial_Effect(Pgrd(10,10),500));
      terrain.AddEffect(new Grid_Effect(Pgrd(0, 0), Pgrd(0, 10)));
      terrain.AddEffect(new Grid_Effect(Pgrd(100, 50), Pgrd(10, 10)));
      terrain.AddEffect(new Grid_Effect(Pgrd(0, 100), Pgrd(10, 0)));
      terrain.AddEffect(new Grid_Effect(Pgrd(-150, -150), Pgrd(3, 10)));
      terrain.AddEffect(new Grid_Effect(Pgrd(150, -150), Pgrd(-3, 10)));
    */
  }

  void doTerrain() {
    if (desired == States::Terrain) {
      auto * major = new LineBuffer();
      auto * minor = new LineBuffer();
      current_config->generate_terrain(major, minor);


      QColor major_c = Qt::green;
      major_c.setAlphaF(.3);

      QColor minor_c = Qt::darkGreen;
      minor_c.setAlphaF(.3);

      emit linebuffer_ready(major, major_c);
      emit linebuffer_ready(minor, minor_c);
    }
  }
  void doNodes() {
    LineBuffer * target = (desired == States::Nodes) ? new LineBuffer() : nullptr;
    current_config->generate_nodes(target);
    
    QColor c = Qt::red;
    c.setAlphaF(.8);

    if (desired == States::Nodes) {
      emit linebuffer_ready(target, c);
    }
  }
  void doClean() {
    LineBuffer * target = (desired == States::Clean) ? new LineBuffer() : nullptr;
    current_config->clean_nodes(target);

    QColor c = Qt::blue;
    c.setAlphaF(.8);

    if (desired == States::Clean) {
      emit linebuffer_ready(target, c);
    }
  }
  void doStructure() {
    auto * target = (desired == States::Structure) ? new LineBuffer() : nullptr;
    current_config->generate_structure(target);
    

    QColor c = Qt::black;
    c.setAlphaF(1);

    if (desired == States::Structure) {
      emit linebuffer_ready(target, c);
    }
  }
  void doRoads() {
    PolygonBuffer * target = (desired == States::Roads) ? new PolygonBuffer() : nullptr;
    PolygonBuffer * errors = (desired == States::Roads) ? new PolygonBuffer() : nullptr;
    current_config->generate_roads(target, errors);
    

    QColor c = Qt::black;
    c.setAlphaF(1);

    QColor c_error = Qt::red;
    c_error.setAlphaF(.8);

    if (desired == States::Roads) {
      emit polygonbuffer_ready(target, c);
      emit polygonbuffer_ready(errors, c_error);
    }
  }
  void doBlocks() {
    PolygonBuffer * target = (desired == States::Blocks) ? new PolygonBuffer() : nullptr;
    current_config->generate_blocks(target);

    QColor c = Qt::blue;
    c.setAlphaF(.3);

    if (desired == States::Blocks) {
      emit polygonbuffer_ready(target, c);
    }
  }
  void doBuildings() {
    PolygonBuffer * target = new PolygonBuffer();
    for(auto block : current_config->getBlocks()){
      const auto proto_border = block->getBorder();
      Pgrd average(0,0);
      for(auto p : proto_border) average += p;
      average /= proto_border.size();
      std::vector<Pgrd> border;
      for(auto p : proto_border) border.push_back(p - average);

      if(Area(border) < 0) continue;

      DCEL system;
      RegionBuffer block_interiors;
      std::list<Pgrd> block_border;//(border.begin(),border.end());
      std::reverse_copy(border.begin(), border.end(), std::back_inserter(block_border));
      block_interiors.push_back(system.region(block_border));
      
      for(int i = 0; i < 10; i++){
        const int j = rand() % border.size();
        const Pgrd root = border[j];
        Pgrd edge = border[(j+1)%border.size()] - border[j];
        const Pgrd mid = root + edge * pRandom(0.0, 1.0);

        edge.Normalize();
        const Pgrd t(edge.Y,-edge.X);
        
        const grd width = pRandom(3.0, 5.0);
        const grd depth = pRandom(5.0, 10.0);
        
        std::list<Pgrd> rect;
        rect.push_back(mid - edge * width);
        rect.push_back(mid - edge * width - t * depth);
        rect.push_back(mid + edge * width - t * depth);
        rect.push_back(mid + edge * width);

        //target->push_back({ rect.begin(),rect.end() });

        RegionBuffer building_interiors = allocateBoundaryFrom(rect, block_interiors);
        for(auto building : building_interiors){
          for (auto face : building->getBounds()) {
            //if(face->area() > 0){
              auto proto_result = face->getLoopPoints();
              std::vector<Pgrd> result;
              for (auto p : proto_result) result.push_back(p + average);
              target->push_back({ result.begin(),result.end() });
           // }
          }
        }
      }
      
    }
    QColor c = Qt::darkBlue;
    c.setAlphaF(1);

    emit polygonbuffer_ready(target, c);
  }

  void doFinished(){
    throw;
  }

private:
  Network* current_config = nullptr;
  States current = States::Reset;
  States desired = States::Reset;
  int seed = 0;
#ifdef DEBUG_INTERSECTIONS
  void run() override {

    //generate the main road, a size between one and four
    //generate a crossing road, with a size less than or equal to the main, optionally terminating or crossing
    //optionally: generate a skew road, with size less than or equal to the crossing road
    srand(time(NULL));
    int x = rand() % 100;
    for(int x = rand() % 100; x>0;--x)
      rand();

    auto * target = new PolygonBuffer();
    Network::runIntersectionTest(target);

    QColor c = Qt::darkGray;
    c.setAlphaF(.8);

    emit polygonbuffer_ready(target, c);
    processing = false;
  }
#else
	void run() override;
#endif

signals:
	void linebuffer_ready(LineBuffer*, QColor);
  void polygonbuffer_ready(PolygonBuffer*, QColor);
  void update_state(int);
	
public:
	bool processing = true;
  bool setState(States _desired, int _seed) {
    const bool reset = seed != _seed || _desired < current;

    if(reset) current = States::Reset;

    desired = _desired;
    seed = _seed;

    return reset;
  }
};

#ifndef DEBUG_INTERSECTIONS
void TRACER_Demo::Data::run() {
  typedef void(TRACER_Demo::Data::*fn)();

  fn process[] = {
    &TRACER_Demo::Data::doReset,
    &TRACER_Demo::Data::doTerrain,
    &TRACER_Demo::Data::doNodes,
    &TRACER_Demo::Data::doClean,
    &TRACER_Demo::Data::doStructure,
    &TRACER_Demo::Data::doRoads,
    &TRACER_Demo::Data::doBlocks,
    &TRACER_Demo::Data::doBuildings,
    &TRACER_Demo::Data::doFinished
  };

  while (current <= desired) {
    emit update_state(current);

    (this->*process[current])();

    ++current;
  }

  processing = false;
}
#endif

TRACER_Demo::~TRACER_Demo() {
	delete d;
}

TRACER_Demo::TRACER_Demo(QWidget *parent)
	: QMainWindow(parent)
	, d(new Data(this))
{
	ui.setupUi(this);

	ui_render = findChild<RenderArea *>(QString("renderWidget"));

	ui_seed = findChild<QSpinBox *>(QString("ui_seed"));
	ui_start = findChild<QPushButton *>(QString("ui_start"));
  ui_state = findChild<QComboBox *>(QString("ui_state"));
  ui_label = findChild<QLabel *>(QString("ui_current"));


  for (States s = Reset; s < Finished; ++s)
    ui_state->addItem(QString(state_names[s]));

	bool res = true;
  
  connect(d, &Data::linebuffer_ready, this, &TRACER_Demo::render_linebuffer);
	connect(d, &Data::polygonbuffer_ready, this, &TRACER_Demo::render_polygonbuffer);
  connect(d, &Data::update_state, this, &TRACER_Demo::update_state);
	res &= !!connect(ui_start, &QPushButton::pressed, this, &TRACER_Demo::start);

	Q_ASSERT(res);

	d->processing = false;
}

void TRACER_Demo::start()
{
  if (d->processing) return;
  d->processing = true;

  if (d->setState(static_cast<States>(ui_state->currentIndex()), ui_seed->value())) {
    ui_render->resetDraw();
  }

  d->start();
}

void TRACER_Demo::render_linebuffer(LineBuffer * packet, QColor c)
{
	ui_render->addLineBuffer(*packet, c);
	delete packet;
}

void TRACER_Demo::render_polygonbuffer(PolygonBuffer * packet, QColor c)
{
  ui_render->addPolygonBuffer(*packet, c);
  delete packet;
}

void TRACER_Demo::update_state(int i) {
  ui_label->setText(QString(state_names[i]));
}

#include "TRACER_Demo.moc"