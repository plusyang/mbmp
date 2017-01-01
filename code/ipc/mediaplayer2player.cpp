/********************* mediaplayer2player.cpp ***************************

Code for the MediaPlayer2.Player interface on DBus.  When registered MBMP
will communicate to other processes.  

Copyright (C) 2013-2016
by: Andrew J. Bibb
License: MIT 

Permission is hereby granted, free of charge, to any person obtaining a copy 
of this software and associated documentation files (the "Software"),to deal 
in the Software without restriction, including without limitation the rights 
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
copies of the Software, and to permit persons to whom the Software is 
furnished to do so, subject to the following conditions: 

The above copyright notice and this permission notice shall be included 
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
DEALINGS IN THE SOFTWARE.
***********************************************************************/  

# include <QtCore/QDebug>
# include <QtDBus/QDBusConnection>
# include <QtDBus/QDBusMessage>

# include "./mediaplayer2player.h"

//  constructor
MediaPlayer2Player::MediaPlayer2Player(Mpris2* parent) : QDBusAbstractAdaptor(parent)
{

	// initialize player properties
	playbackstatus = QString("Stopped");
	loopstatus = QString("None");
	playbackrate = 1.0;
	shuffle = false;
	metadata.clear();
	volume = 0.0;
	position = 0.0;
	minimumrate = 1.0;
	maximumrate = 1.0;
	cangonext = false;
	cangoprevious = false;
	canplay = false;
	canpause = false;
	canseek = false;
	cancontrol = true;
	
	// data members
	changeditems.clear();	
	
}
 
/////////////////////// Public Functions //////////////////////////////
//
// Function to set the Playback status. 
// dbus read only
void MediaPlayer2Player::setPlaybackStatus(const QString& s_ps)
{
	const QStringList valid{"Playing", "Paused", "Stopped"};
	
	// return if no change
	if (playbackstatus == s_ps) return;
	
	// return if not valid
	if (! valid.contains(s_ps, Qt::CaseSensitive) ) return;
	
	// changed and valid
	playbackstatus = s_ps;
	changeditems.append(MBMP_MPRIS::PlaybackStatus);
	sendPropertyChanged();
}

//
// Function to set the Loop status. 
// dbus read/write
void MediaPlayer2Player::setLoopStatus(const QString& s_ls)
{
	const QStringList valid{"None", "Track", "Playlist"};
	
	// return if no change
	if (loopstatus == s_ls) return;
	
	// return if not valid
	if (! valid.contains(s_ls, Qt::CaseSensitive) ) return;
	
	// MBMP does not support Track repeat, return if set (via dbus interface)
	if (s_ls == "Track") return;
	
	// changed and valid
	loopstatus = s_ls;
	changeditems.append(MBMP_MPRIS::LoopStatus);
	sendPropertyChanged();
	
	// let MBMP know we've changed (change could have been via the mpris2 dbus interface)
	if (s_ls == "Playlist")
		static_cast<Mpris2*>(this->parent())->emitLoopStatusChanged(true);
	else
		static_cast<Mpris2*>(this->parent())->emitLoopStatusChanged(false);
}

//
// Function to set the Shuffle property. 
// dbus read/write
void MediaPlayer2Player::setShuffle(const bool& b_s)
{
	// return if no change
	if (shuffle == b_s) return;
	
	// changed and valid
	shuffle = b_s;
	changeditems.append(MBMP_MPRIS::Shuffle);
	sendPropertyChanged();
}

//
// Function to set the Shuffle property. 
// dbus read/write
void MediaPlayer2Player::setVolume(const double& d_v)
{
	// return if no change
	if (volume == d_v) return;
	
	// volume changed
	if (d_v < 0.0 ) volume = 0.0;
		else if (d_v > 1.0) volume = 1.0;
			else volume = d_v;	
	
	// let mpris2 things know we've changed
	changeditems.append(MBMP_MPRIS::Volume);
	sendPropertyChanged();

	// let MBMP know we've changed (change could have been via the mpris2 dbus interface)
	static_cast<Mpris2*>(this->parent())->emitVolumeChanged(static_cast<int>(d_v * 30.0 + 0.5) );
}

/////////////////////// Private Functions //////////////////////////////
//
// Function to emit the org.freedesktop.Dbus.Properties.PropertiesChanged()
// DBus signal.  Called from the local inline setxxx functions
void MediaPlayer2Player::sendPropertyChanged()
{
	// changed properties
	if (changeditems.size() <= 0) return;  

	// create a map of all changed items. Use for loop because it is possible
	// (but highly unlikely) that this slot could be called with more than 
	// one property needing to be changed 
  QMap<QString, QVariant> vmap;
  for (int i = 0; i < changeditems.size(); ++i) {
		switch (changeditems.at(i) ) {
			case MBMP_MPRIS::PlaybackStatus:
				vmap["PlaybackSatus"] = QVariant(playbackstatus);
				break;
			case MBMP_MPRIS::LoopStatus:
				vmap["LoopSatus"] = QVariant(loopstatus);
				break;
			case MBMP_MPRIS::PlaybackRate:
				vmap["Rate"] = QVariant(playbackrate);
				break;
			case MBMP_MPRIS::Shuffle:
				vmap["Shuffle"] = QVariant(shuffle);
				break;
			case MBMP_MPRIS::Metadata:
				vmap["Metadata"] = QVariant(metadata);
				break;
			case MBMP_MPRIS::Volume:
				vmap["Volume"] = QVariant(volume);
				break;
			case MBMP_MPRIS::MinRate:
				vmap["MinimumRate"] = QVariant(minimumrate);
				break;
			case MBMP_MPRIS::MaxRate:
				vmap["MaximumRate"] = QVariant(maximumrate);
				break;
			case MBMP_MPRIS::CanGoNext:
				vmap["CanGoNext"] = QVariant(cangonext);
				break;
			case MBMP_MPRIS::CanGoPrevious:
				vmap["CanGoPrevious"] = QVariant(cangoprevious);
				break;
			case MBMP_MPRIS::CanPlay:
				vmap["CanPlay"] = QVariant(canplay);
				break;	
			case MBMP_MPRIS::CanPause:
				vmap["CanPause"] = QVariant(canpause);
				break;
			case MBMP_MPRIS::CanSeek:
				vmap["CanSeek"] = QVariant(canseek);
				break;
			default:
				break;
		}	// switch				
  }	// for
  changeditems.clear();
  
  // create the message.  We never remove a property so we don't need
  // to deal with that - send an empty qstringlist
  QList<QVariant> vlist;
  vlist << QVariant(IPC_INTERFACE_MEDIAPLAYER2PLAYER) << vmap << QStringList();
  QDBusMessage msg = QDBusMessage::createSignal(IPC_OBJECT, "org.freedesktop.Dbus.Properties", "PropertiesChanged");  
  msg.setArguments(vlist);																																														
	QDBusConnection::sessionBus().send(msg);
	
	return;
}    

