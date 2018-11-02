#include "AppUI.h"

#include "Paths.h"

using namespace reza::ui;
using namespace reza::glsl;
using namespace reza::paths;
using namespace std;
using namespace ci;
using namespace ci::app;

namespace reza {
namespace app {

AppUI::~AppUI()
{
}

bool AppUI::isHit( const ivec2 &input )
{
#if USE_WINDOW_CANVAS
	return false;
#else
	for( auto &it : mUIs ) {
		bool hit = it.second->isHit( input );
		if( hit ) {
			return true;
		}
	}
#endif
	return false;
}

void AppUI::minimize()
{
#if USE_WINDOW_CANVAS
	return;
#else
	for( auto &it : mUIs ) {
		it.second->setMinified( true );
	}
#endif
}

void AppUI::maximize()
{
#if USE_WINDOW_CANVAS
	return;
#else
	for( auto &it : mUIs ) {
		it.second->setMinified( false );
	}
#endif
}

void AppUI::arrange()
{
#if USE_WINDOW_CANVAS
	return;
#else
	UIPanelRef last = nullptr;
	for( auto &it : mUIs ) {
		auto ui = it.second;

		float paddingLeft = ui->getPadding().mLeft;
		float paddingTop = ui->getPadding().mTop;

		if( last != nullptr ) {
			auto originLast = last->getOrigin();
			auto sizeLast = last->getSize();

			vec2 originCurrent = vec2( originLast.x, originLast.y + sizeLast.y + paddingTop );
			ui->setOrigin( originCurrent );

			if( ( originCurrent.y + ui->getSize().y ) > ui->getWindow()->getSize().y ) {
				ui->setOrigin( vec2( originLast.x + sizeLast.x + paddingLeft, paddingTop ) );
			}
		}
		else {
			ui->setOrigin( vec2( paddingLeft, paddingTop ) );
		}
		last = ui;
	}

#endif
}

UIPanelRef AppUI::getUI( const string &name )
{
	auto it = mUIs.find( name );
	if( it != mUIs.end() ) {
		return it->second;
	}
	return nullptr;
}

//UI METHODS
UIPanelRef AppUI::addUI( UIPanelRef ui )
{
	ui->autoSizeToFitSubviews();
	mUIs[ui->getName()] = ui;
	return ui;
}

UIPanelRef AppUI::removeUI( UIPanelRef ui )
{
	mUIs.erase( ui->getName() );
	return ui;
}

UIPanelRef AppUI::setupUI( std::string name, const std::function<UIPanelRef( UIPanelRef )> &cb )
{
	auto createUI = []( string name_ ) {
		auto ui = UIPanel::create( name_ );
		ui->setPadding( Paddingf( 2 ) );
		return ui;
	};
	return addUI( cb( createUI( name ) ) );
}

UIPanelRef AppUI::addShaderParamsUI( UIPanelRef &ui, GlslParams &glslParams )
{
	auto calculatePrecision = []( float value ) {
		std::ostringstream out;
		out << value;
		string num = out.str();
		size_t found = num.find( "." );
		int len = 0;
		if( found != string::npos ) {
			len = static_cast<int>( num.size() ) - 1 - static_cast<int>( found );
		}
		return len;
	};
	/*
	uniform vec3 axis0;     //multi:0.0,1.0,0.5         -> multi
	uniform vec2 axis1;     //multi:0.0,1.0,0.5         -> multi
	uniform int iaxis;      //slider:0.0,1.0,0.5        -> slider
	uniform int axis3;      //dialer:0.0,1.0,0.5        -> dialer
	uniform float axis2;    //slider:0.0,1.0,0.5        -> slider
	uniform float axis3;    //dialer:0.0,1.0,0.5        -> dialer
	uniform float legacy;   //ui:0.0,1.0,0.5            -> multi
	uniform vec2 range2;    //range:0.0,1.0,0.2,0.75    -> range
	uniform vec2 pad;       //pad:-1.0,1.0,0.0          -> pad
	uniform bool state0;    //button:0                  -> button
	uniform bool state1;    //toggle:1                  -> toggle
	*/

	MultiSliderRef ref = nullptr;
	vector<MultiSlider::Data> data;

	auto &order = glslParams.getParamOrder();
	auto &types = glslParams.getTypeMap();

	auto &cp = glslParams.getColorParams();

	auto &bp = glslParams.getBoolParams();

	auto &ip = glslParams.getIntParams();
	auto &ir = glslParams.getIntRanges();

	auto &fp = glslParams.getFloatParams();
	auto &fr = glslParams.getFloatRanges();

	auto &v2p = glslParams.getVec2Params();
	auto &v2r = glslParams.getVec2Ranges();

	auto &v3p = glslParams.getVec3Params();
	auto &v3r = glslParams.getVec3Ranges();

	auto &v4p = glslParams.getVec4Params();
	auto &v4r = glslParams.getVec4Ranges();

	for( auto &it : order ) {
		string name = it.second;
		string type = types.at( name ).first; // float
		string uitype = types.at( name ).second; // slider
		if( type == "bool" ) {
			bool *ptr = &bp[name];
			if( uitype == "button" ) {
				ui->addButton( name, ptr );
			}
			else if( uitype == "toggle" ) {
				ui->addToggle( name, ptr );
			}
		}
		else if( type == "int" ) {
			int *ptr = &ip[name];
			int low = ir[name].first;
			int high = ir[name].second;
			if( uitype == "slider" ) {
				ui->addSlideri( name, ptr, low, high );
			}
			else if( uitype == "dialer" ) {
				ui->addDialeri( name, ptr, low, high );
			}
		}
		else if( type == "float" ) {
			float *ptr = &fp[name];
			float low = fr[name].first;
			float high = fr[name].second;
			if( uitype == "slider" ) {
				ui->addSliderf( name, &fp.at( name ), low, high );
			}
			else if( uitype == "dialer" ) {
				auto dfmt = Dialerf::Format().precision( std::max( calculatePrecision( *ptr ), 5 ) );
				ui->addDialerf( name, ptr, low, high, dfmt );
			}
			else if( uitype == "ui" ) {
				data.emplace_back( MultiSlider::Data( name, ptr, low, high ) );
			}
		}
		else if( type == "vec2" ) {
			vec2 *ptr = &v2p[name];
			float low = v2r[name].first;
			float high = v2r[name].second;
			if( uitype == "range" ) {
				ui->addRangef( name, &ptr->x, &ptr->y, low, high );
			}
			else if( uitype == "pad" ) {
				ui->addXYPad( name, ptr, XYPad::Format().min( vec2( low, high ) ).max( vec2( high, low ) ) );
			}
			else if( uitype == "ui" ) {
				ui->addMultiSlider( name, { MultiSlider::Data( name + "-X", &ptr->x, low, high ), MultiSlider::Data( name + "-Y", &ptr->y, low, high ) } );
			}
			else if( uitype == "slider" ) {
				ui->addSliderf( name + "-X", &ptr->x, low, high );
				ui->addSliderf( name + "-Y", &ptr->y, low, high );
			}
			else if( uitype == "dialer" ) {
				ui->addSpacer();
				auto fmt = Dialerf::Format().fontSize( FontSize::SMALL ).precision( std::max( calculatePrecision( ptr->x ), 1 ) ).label( false );
				auto lb = ui->addLabel( name, FontSize::SMALL );
				vec2 lbo = lb->getOrigin( false );
				ui->right();
				ui->addDialerf( "X", &ptr->x, low, high, fmt );
				ui->addDialerf( "Y", &ptr->y, low, high, fmt );
				lb->setOrigin( lbo + vec2( 0.0, lb->getPadding().mTop ) );
				ui->down();
				ui->addSpacer();
			}
		}
		else if( type == "vec3" ) {
			vec3 *ptr = &v3p[name];
			float low = v3r[name].first;
			float high = v3r[name].second;
			if( uitype == "ui" ) {
				ui->addMultiSlider( name, { MultiSlider::Data( name + "-X", &ptr->x, low, high ), MultiSlider::Data( name + "-Y", &ptr->y, low, high ), MultiSlider::Data( name + "-Z", &ptr->z, low, high ) } );
			}
			else if( uitype == "slider" ) {
				ui->addSliderf( name + "-X", &ptr->x, low, high );
				ui->addSliderf( name + "-Y", &ptr->y, low, high );
				ui->addSliderf( name + "-Z", &ptr->z, low, high );
			}
			else if( uitype == "dialer" ) {
				ui->addSpacer();
				auto fmt = Dialerf::Format().fontSize( FontSize::SMALL ).precision( std::max( calculatePrecision( ptr->x ), 1 ) ).label( false );
				auto lb = ui->addLabel( name, FontSize::SMALL );
				vec2 lbo = lb->getOrigin( false );
				ui->right();
				ui->addDialerf( "X", &ptr->x, low, high, fmt );
				ui->addDialerf( "Y", &ptr->y, low, high, fmt );
				ui->addDialerf( "Z", &ptr->z, low, high, fmt );
				lb->setOrigin( lbo + vec2( 0.0, lb->getPadding().mTop ) );
				ui->down();
				ui->addSpacer();
			}
		}
		else if( type == "vec4" ) {
			if( uitype == "ui" ) {
				vec4 *ptr = &v4p[name];
				float low = v4r[name].first;
				float high = v4r[name].second;
				ui->addMultiSlider( name, { MultiSlider::Data( name + "-X", &ptr->x, low, high ), MultiSlider::Data( name + "-Y", &ptr->y, low, high ), MultiSlider::Data( name + "-Z", &ptr->z, low, high ), MultiSlider::Data( name + "-W", &ptr->w, low, high ) } );
			}
			else if( uitype == "slider" ) {
				vec4 *ptr = &v4p[name];
				float low = v4r[name].first;
				float high = v4r[name].second;
				ui->addSliderf( name + "-X", &ptr->x, low, high );
				ui->addSliderf( name + "-Y", &ptr->y, low, high );
				ui->addSliderf( name + "-Z", &ptr->z, low, high );
				ui->addSliderf( name + "-W", &ptr->w, low, high );
			}
			else if( uitype == "dialer" ) {
				ui->addSpacer();
				vec4 *ptr = &v4p[name];
				float low = v4r[name].first;
				float high = v4r[name].second;
				auto fmt = Dialerf::Format().fontSize( FontSize::SMALL ).precision( std::max( calculatePrecision( ptr->x ), 1 ) ).label( false );
				auto lb = ui->addLabel( name, FontSize::SMALL );
				vec2 lbo = lb->getOrigin( false );
				ui->right();
				ui->addDialerf( name + "-X", &ptr->x, low, high, fmt );
				ui->addDialerf( name + "-Y", &ptr->y, low, high, fmt );
				ui->addDialerf( name + "-Z", &ptr->z, low, high, fmt );
				ui->addDialerf( name + "-W", &ptr->w, low, high, fmt );
				lb->setOrigin( lbo + vec2( 0.0, lb->getPadding().mTop ) );
				ui->down();
				ui->addSpacer();
			}
			else if( uitype == "color" ) {
				ui->addColorPicker( name, &cp[name] );
			}
		}
	}

	if( data.size() ) {
		ui->addMultiSlider( "UNIFORMS", data );
	}

	ui->addSpacer()->setDrawFill( false );
	ui->autoSizeToFitSubviews();
	return ui;
}

void AppUI::loadUIs( const fs::path &path )
{
	for( auto &it : mUIs ) {
		loadUI( it.second, path );
	}
}

void AppUI::saveUIs( const fs::path &path )
{
	for( auto &it : mUIs ) {
		saveUI( it.second, path );
	}
}

void AppUI::spawnUIs()
{
	for( auto &it : mUIs ) {
#if USE_WINDOW_CANVAS
		it.second->spawn();
#else
		it.second->enable();
#endif
	}
}

void AppUI::toggleUIs()
{
	for( auto &it : mUIs ) {
		toggleUI( it.first );
	}
}

void AppUI::toggleUI( const string &name )
{
	auto ui = getUI( name );
	if( ui != nullptr ) {
#if USE_WINDOW_CANVAS
		if( ui->isValid() ) {
			ui->close();
		}
		else {
			ui->spawn();
		}
#else
		ui->setEnabled( !ui->isEnabled() );
#endif
	}
}

void AppUI::spawnUI( const string &name )
{
	auto ui = getUI( name );
	if( ui != nullptr ) {
#if USE_WINDOW_CANVAS
		ui->spawn();
#else
		ui->enable();
#endif
	}
}

void AppUI::loadUI( UIPanelRef ui, const fs::path &path )
{
	ui->load( addPath( path, ui->getName() + ".json" ) );
}

void AppUI::saveUI( UIPanelRef ui, const fs::path &path )
{
	ui->save( addPath( path, ui->getName() + ".json" ) );
}

} // namespace app
} // namespace reza
