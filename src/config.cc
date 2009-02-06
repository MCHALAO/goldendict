/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "config.hh"
#include <QDir>
#include <QFile>
#include <QtXml>

namespace Config {

namespace
{
  QDir getHomeDir()
  {
    QDir result = QDir::home();

    char const * pathInHome = 
      #ifdef Q_OS_WIN32
      "Application Data/GoldenDict"
      #else
      ".goldendict"
      #endif
      ;

    result.mkpath( pathInHome );

    if ( !result.cd( pathInHome ) )
      throw exCantUseHomeDir();

    return result;
  }

  QString getConfigFileName()
  {
    return getHomeDir().absoluteFilePath( "config" );
  }
}

Preferences::Preferences():
  enableTrayIcon( false ),
  startToTray( false ),
  enableScanPopup( false ),
  enableScanPopupModifiers( false ),
  scanPopupModifiers( 0 )
{
}


Class load() throw( exError )
{
  QString configName  = getConfigFileName();

  if ( !QFile::exists( configName ) )
  {
    // Make the default config, save it and return it
    Class c;

    #ifdef Q_OS_LINUX

    if ( QDir( "/usr/share/stardict/dic" ).exists() )
      c.paths.push_back( "/usr/share/stardict/dic" );

    #endif

    save( c );

    return c;
  }

  QFile configFile( configName );

  if ( !configFile.open( QFile::ReadOnly ) )
    throw exCantReadConfigFile();

  QDomDocument dd;

  QString errorStr;
  int errorLine, errorColumn;

  if ( !dd.setContent( &configFile, false, &errorStr, &errorLine, &errorColumn  ) )
  {
    printf( "Error: %s at %d,%d\n", errorStr.toLocal8Bit().constData(),  errorLine,  errorColumn );
      throw exMalformedConfigFile();
  }

  configFile.close();

  QDomNode root = dd.namedItem( "config" );

  Class c;

  QDomNode paths = root.namedItem( "paths" );

  if ( !paths.isNull() )
  {
    QDomNodeList nl = paths.toElement().elementsByTagName( "path" );

    for( unsigned x = 0; x < nl.length(); ++x )
      c.paths.push_back( nl.item( x ).toElement().text() );
  }

  QDomNode groups = root.namedItem( "groups" );

  if ( !groups.isNull() )
  {
    QDomNodeList nl = groups.toElement().elementsByTagName( "group" );

    for( unsigned x = 0; x < nl.length(); ++x )
    {
      QDomElement grp = nl.item( x ).toElement();

      Group g;

      g.name = grp.attribute( "name" );
      g.icon = grp.attribute( "icon" );

      QDomNodeList dicts = grp.elementsByTagName( "dictionary" );

      for( unsigned y = 0; y < dicts.length(); ++y )
        g.dictionaries.push_back( dicts.item( y ).toElement().text() );

      c.groups.push_back( g );
    }
  }

  QDomNode preferences = root.namedItem( "preferences" );

  if ( !preferences.isNull() )
  {
    c.preferences.enableTrayIcon = ( preferences.namedItem( "enableTrayIcon" ).toElement().text() == "1" );
    c.preferences.startToTray = ( preferences.namedItem( "startToTray" ).toElement().text() == "1" );
    c.preferences.closeToTray = ( preferences.namedItem( "closeToTray" ).toElement().text() == "1" );
    c.preferences.enableScanPopup = ( preferences.namedItem( "enableScanPopup" ).toElement().text() == "1" );
    c.preferences.enableScanPopupModifiers = ( preferences.namedItem( "enableScanPopupModifiers" ).toElement().text() == "1" );
    c.preferences.scanPopupModifiers = ( preferences.namedItem( "scanPopupModifiers" ).toElement().text().toULong() );    
  }

  c.lastMainGroup = root.namedItem( "lastMainGroup" ).toElement().text();
  c.lastPopupGroup = root.namedItem( "lastPopupGroup" ).toElement().text();

  return c;
}

void save( Class const & c ) throw( exError )
{
  QFile configFile( getConfigFileName() );

  if ( !configFile.open( QFile::WriteOnly ) )
    throw exCantWriteConfigFile();

  QDomDocument dd;

  QDomElement root = dd.createElement( "config" );
  dd.appendChild( root );

  {
    QDomElement paths = dd.createElement( "paths" );
    root.appendChild( paths );
  
    for( Paths::const_iterator i = c.paths.begin(); i != c.paths.end(); ++i )
    {
      QDomElement path = dd.createElement( "path" );
      paths.appendChild( path );
  
      QDomText value = dd.createTextNode( *i );
  
      path.appendChild( value );
    }
  }

  {
    QDomElement groups = dd.createElement( "groups" );
    root.appendChild( groups );

    for( Groups::const_iterator i = c.groups.begin(); i != c.groups.end(); ++i )
    {
      QDomElement group = dd.createElement( "group" );
      groups.appendChild( group );

      QDomAttr name = dd.createAttribute( "name" );

      name.setValue( i->name );

      group.setAttributeNode( name );

      if ( i->icon.size() )
      {
        QDomAttr icon = dd.createAttribute( "icon" );

        icon.setValue( i->icon );

        group.setAttributeNode( icon );
      }

      for( vector< QString >::const_iterator j = i->dictionaries.begin(); j != i->dictionaries.end(); ++j )
      {
        QDomElement dictionary = dd.createElement( "dictionary" );

        group.appendChild( dictionary );

        QDomText value = dd.createTextNode( *j );

        dictionary.appendChild( value );
      }
    }
  }

  {
    QDomElement preferences = dd.createElement( "preferences" );
    root.appendChild( preferences );

    QDomElement opt = dd.createElement( "enableTrayIcon" );
    opt.appendChild( dd.createTextNode( c.preferences.enableTrayIcon ? "1":"0" ) );
    preferences.appendChild( opt );

    opt = dd.createElement( "startToTray" );
    opt.appendChild( dd.createTextNode( c.preferences.startToTray ? "1":"0" ) );
    preferences.appendChild( opt );

    opt = dd.createElement( "closeToTray" );
    opt.appendChild( dd.createTextNode( c.preferences.closeToTray ? "1":"0" ) );
    preferences.appendChild( opt );

    opt = dd.createElement( "enableScanPopup" );
    opt.appendChild( dd.createTextNode( c.preferences.enableScanPopup ? "1":"0" ) );
    preferences.appendChild( opt );

    opt = dd.createElement( "enableScanPopupModifiers" );
    opt.appendChild( dd.createTextNode( c.preferences.enableScanPopupModifiers ? "1":"0" ) );
    preferences.appendChild( opt );

    opt = dd.createElement( "scanPopupModifiers" );
    opt.appendChild( dd.createTextNode( QString::number( c.preferences.scanPopupModifiers ) ) );
    preferences.appendChild( opt );
  }

  {
    QDomElement opt = dd.createElement( "lastMainGroup" );
    opt.appendChild( dd.createTextNode( c.lastMainGroup ) );
    root.appendChild( opt );

    opt = dd.createElement( "lastPopupGroup" );
    opt.appendChild( dd.createTextNode( c.lastPopupGroup ) );
    root.appendChild( opt );
  }

  configFile.write( dd.toByteArray() );
}

QString getIndexDir() throw( exError )
{
  QDir result = getHomeDir();

  result.mkpath( "index" );

  if ( !result.cd( "index" ) )
    throw exCantUseIndexDir();

  return result.path() + QDir::separator();
}

QString getUserCssFileName() throw( exError )
{
  return getHomeDir().filePath( "style.css" );
}

QString getUserQtCssFileName() throw( exError )
{
  return getHomeDir().filePath( "qt-style.css" );
}

}