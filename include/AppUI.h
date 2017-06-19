/*
Copyright(c) 2017 Reza Ali syed.reza.ali@gmail.com www.syedrezaali.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <functional>
#include <map>

#include "cinder/app/Window.h"

#include "GlslParams.h"
#include "UI.h"

#define USE_WINDOW_CANVAS 0
#if USE_WINDOW_CANVAS  
typedef reza::ui::WindowCanvas UIPanel;
typedef reza::ui::WindowCanvasRef UIPanelRef;
#else 
typedef reza::ui::SuperCanvas UIPanel;
typedef reza::ui::SuperCanvasRef UIPanelRef;
#endif

namespace reza {
namespace app {
typedef std::shared_ptr<class AppUI> AppUIRef;
class AppUI {
  public:
	static AppUIRef create()
	{
		return AppUIRef( new AppUI() );
	}
	virtual ~AppUI();

	bool isHit(const ci::ivec2& input); 

	void minimize(); 
	void maximize(); 
	void arrange(); 

	//UI METHODS
	UIPanelRef getUI( const std::string &name );
	UIPanelRef addUI( UIPanelRef ui );
	UIPanelRef removeUI( UIPanelRef ui );

	UIPanelRef setupUI( std::string name, const std::function<UIPanelRef( UIPanelRef )> &cb );
	UIPanelRef addShaderParamsUI( UIPanelRef &ui, reza::glsl::GlslParams &glslParams );

	void loadUIs( const ci::fs::path &path );
	void saveUIs( const ci::fs::path &path );
	void spawnUIs();
	void toggleUIs();
	void toggleUI( const std::string &name );
	void spawnUI( const std::string &name );

	void loadUI( UIPanelRef ui, const ci::fs::path &path );
	void saveUI( UIPanelRef ui, const ci::fs::path &path );

	std::map<std::string, UIPanelRef> mUIs;


};
} // namespace app
} // namespace reza