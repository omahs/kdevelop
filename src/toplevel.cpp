#include <kapplication.h>
#include <kconfig.h>


#include "toplevel.h"
#include "core.h"

//#define NEWMAINWINDOW

#ifdef NEWMAINWINDOW
#include "newmainwindow.h"
#else
#include "mainwindow.h"
#endif

KDevMainWindow *TopLevel::s_instance = 0;


bool TopLevel::mainWindowValid()
{
  return s_instance != 0;
}

KDevMainWindow *TopLevel::getInstance()
{
  if (!s_instance)
  {

	KConfig *config = kapp->config();
	config->setGroup("UI");
	int mdimode = config->readNumEntry("MDIMode", KMdi::IDEAlMode);
    
#ifdef NEWMAINWINDOW
	NewMainWindow *mainWindow = new NewMainWindow( 0, "MainWindow", (KMdi::MdiMode)mdimode );
#else
    MainWindow *mainWindow = new MainWindow( 0, "MainWindow", (KMdi::MdiMode)mdimode );
#endif

    s_instance = mainWindow;

    mainWindow->init();

    kapp->setMainWidget(mainWindow);
  }

  return s_instance;
}

void TopLevel::invalidateInstance(KDevMainWindow *instance)
{
  if ( s_instance == instance )
    s_instance = 0;
}
