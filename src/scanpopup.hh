/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __SCANPOPUP_HH_INCLUDED__
#define __SCANPOPUP_HH_INCLUDED__

#include "article_netmgr.hh"
#include "articleview.hh"
#include "wordfinder.hh"
#include "keyboardstate.hh"
#include "config.hh"
#include "ui_scanpopup.h"
#include <QDialog>
#include <QClipboard>

/// This is a popup dialog to show translations when clipboard scanning mode
/// is enabled.
class ScanPopup: public QDialog, KeyboardState
{
  Q_OBJECT

public:

  ScanPopup( QWidget * parent,
             Config::Class & cfg,
             ArticleNetworkAccessManager &,
             std::vector< sptr< Dictionary::Class > > const & allDictionaries,
             Instances::Groups const & );

private:

  Config::Class & cfg;
  std::vector< sptr< Dictionary::Class > > const & allDictionaries;
  Instances::Groups const & groups;
  Ui::ScanPopup ui;
  ArticleView * definition;
  QString inputWord;
  WordFinder wordFinder;

  vector< QString > diacriticMatches, prefixMatches;

  bool mouseEnteredOnce;

  void handleInputWord( QString const & );
  void initiateTranslation();

  vector< sptr< Dictionary::Class > > const & getActiveDicts();

  virtual void mouseMoveEvent( QMouseEvent * event );
  virtual void leaveEvent( QEvent * event );

  void popupWordlist( vector< QString > const &, QToolButton * button );

  /// Returns inputWord, chopped with appended ... if it's too long/
  QString elideInputWord();

private slots:

  void clipboardChanged( QClipboard::Mode );
  void mouseHovered( QString const & );
  void currentGroupChanged( QString const & );
  void prefixMatchComplete( WordFinderResults r );
  void diacriticButtonClicked();
  void prefixButtonClicked();
  void initialWordClicked();
  void pinButtonClicked( bool checked );
};

#endif