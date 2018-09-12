/*******************************************************************************
**                                                                            **
**                     Jiedi(China nanjing)Ltd.                               **
**	               �������Ĳܿ����˴��������Ϊѧϰ�ο�                       **
*******************************************************************************/

/*****************************FILE INFOMATION***********************************
**
** Project       : FFmpeg
** Description   : FFMPEG��Ŀ����ʾ��
** Contact       : xiacaojun@qq.com
**        ����   : http://blog.csdn.net/jiedichina
**		��Ƶ�γ�
**�����ƿ���	http://study.163.com/u/xiacaojun
**��Ѷ����		https://jiedi.ke.qq.com/
**csdnѧԺ		http://edu.csdn.net/lecturer/lecturer_detail?lecturer_id=961
**51ctoѧԺ	    http://edu.51cto.com/lecturer/index/user_id-12016059.html
**�������µ�ffmpeg�汾 http://www.ffmpeg.club
**
**   ffmpeg+qt������ ѧԱȺ ��462249121 ����Ⱥ���ش���ͽ���
**   ΢�Ź��ں�  : jiedi2007
**		ͷ����	 : �Ĳܿ�
**
*******************************************************************************/
//������������������ ѧԱ��Ⱥ462249121���ش���ͽ���

#include "XPlay2.h"
#include <QFileDialog>
#include <QDebug>
#include "XDemuxThread.h"
#include <QMessageBox>
static XDemuxThread dt;
XPlay2::XPlay2(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	dt.Start();
	//char *url = "rtmp://live.hkstv.hk.lxdns.com/live/hks";
	startTimer(40);
	
	
}
XPlay2::~XPlay2()
{
	dt.Close(); 
}	
void XPlay2::SliderPress()
{
	isSliderPress = true;
}
void XPlay2::SliderRelease()
{
	isSliderPress = false;
	double pos = 0.0;
	pos = (double)ui.playPos->value() / (double)ui.playPos->maximum();
	dt.Seek(pos);
}
//��ʱ�� ��������ʾ
void XPlay2::timerEvent(QTimerEvent *e)
{
	if (isSliderPress)return;
	long long total = dt.totalMs;
	if (total > 0)
	{
		double pos = (double)dt.pts / (double)total;
		int v = ui.playPos->maximum() * pos;
		ui.playPos->setValue(v);
	}
}
//˫��ȫ��
void XPlay2::mouseDoubleClickEvent(QMouseEvent *e)
{
	if (isFullScreen())
		this->showNormal();
	else
		this->showFullScreen();
}
//���ڳߴ�仯
void XPlay2::resizeEvent(QResizeEvent *e)
{
	ui.playPos->move(50, this->height() - 100);
	ui.playPos->resize(this->width() - 100, ui.playPos->height());
	ui.openFile->move(100,this->height() - 150);
	ui.isplay->move(ui.openFile->x() + ui.openFile->width() + 10, ui.openFile->y());
	ui.video->resize(this->size());
}

void XPlay2::PlayOrPause()
{
	bool isPause = !dt.isPause;
	SetPause(isPause);
	dt.SetPause(isPause);
	
}

void XPlay2::SetPause(bool isPause)
{
	if (isPause)
		ui.isplay->setText(QString::fromLocal8Bit("�� ��"));
	else
		ui.isplay->setText(QString::fromLocal8Bit("�� ͣ"));
}



void XPlay2::OpenFile()
{
	//ѡ���ļ�
	QString name = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("ѡ����Ƶ�ļ�"));
	if (name.isEmpty())return;
	this->setWindowTitle(name);
	if (!dt.Open(name.toLocal8Bit(), ui.video))
	{
		QMessageBox::information(0, "error", "open file failed!");
		return;
	}
	SetPause(dt.isPause);

}