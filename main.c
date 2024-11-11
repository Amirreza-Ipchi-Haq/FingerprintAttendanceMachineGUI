#include<gtk/gtk.h>
#include<stdio.h>
#include"dynastr.h"
#include<time.h>
char *port=0,isSerialOpen=0,isTempWindowOpen=0;
GtkWidget *window,*entry,*tempWindow,*(button[12]),*(spinbutton[6]);
void enable(char condition){
	gtk_widget_set_sensitive(button[1],condition),
	gtk_widget_set_sensitive(button[2],condition),
	gtk_widget_set_sensitive(button[3],condition),
	gtk_widget_set_sensitive(button[4],condition),
	gtk_widget_set_sensitive(button[5],condition),
	gtk_widget_set_sensitive(button[6],condition),
	gtk_widget_set_sensitive(button[7],condition),
	gtk_widget_set_sensitive(button[8],condition),
	gtk_widget_set_sensitive(button[9],condition),
	gtk_widget_set_sensitive(button[10],condition),
	gtk_widget_set_sensitive(button[11],condition);
	return;
}
#ifdef _WIN32
#include<windows.h>
#define SLEEP(s) Sleep(s)
HANDLE serial;
DWORD bytesWritten,bytesRead;
void init(){
	SetEnvironmentVariable("GSK_RENDERER","gl");
	return;
}
void closeSerial(){
	if(isSerialOpen)
		CloseHandle(serial),isSerialOpen=0;
	return;
}
void error(){
	if(isTempWindowOpen)
		gtk_window_destroy(GTK_WINDOW(tempWindow)),isTempWindowOpen=0;
	if(port)
		free(port),port=0;
	closeSerial(),enable(0);
	return;
}
void setup(){
	if(strlen(port)<4){
		error();
		return;
	}
	{
		char s[5]={port[0],port[1],port[2],port[3],0};
		if(strcmp(s,"\\\\.\\"))
			port=dynastr_strappend("\\\\.\\",port,1);
	}
	DCB dcbSerialParams={0};
	COMMTIMEOUTS timeouts={0};
	serial=CreateFile(port,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
	if(serial==INVALID_HANDLE_VALUE){
		error();
		return;
	}
	isSerialOpen=1,dcbSerialParams.DCBlength=sizeof(dcbSerialParams);
	if(!GetCommState(serial,&dcbSerialParams)){
		error();
		return;
	}
	if(!GetCommTimeouts(serial,&timeouts)){
		error();
		return;
	}
	dcbSerialParams.BaudRate=CBR_9600;
	dcbSerialParams.ByteSize=8;
	dcbSerialParams.StopBits=ONESTOPBIT;
	dcbSerialParams.Parity=NOPARITY;
	if(!SetCommState(serial,&dcbSerialParams)){
		error();
		return;
	}
	timeouts.ReadIntervalTimeout=50;
	timeouts.ReadIntervalTimeout=50;
	timeouts.ReadTotalTimeoutMultiplier=10;
	timeouts.WriteTotalTimeoutConstant=50;
	timeouts.WriteTotalTimeoutMultiplier=10;
	if(!SetCommTimeouts(serial,&timeouts)){
		error();
		return;
	}
	enable(1);
	return;
}
void writeSerial(char *str){
	if(!WriteFile(serial,str,strlen(str),&bytesWritten,NULL))
		error();
}
char *readSerial(){
	char *s=dynastr_strtmp("",0),c[1];
	for(unsigned t=0;1;){
		if(!ReadFile(serial,c,1,&bytesRead,NULL)){
			free(s),error();
			return dynastr_strtmp("",0);
		}
		if(bytesRead)
			t=0,s=dynastr_strappend(s,DYNASTR_CHR2STR(*c),2);
		else{
			t++;
			if(t>200)
				break;
		}
	}
	return s;
}
#else
#include<fcntl.h>
#include<unistd.h>
#define SLEEP(s) usleep(s*1000)
#define PRINTABLE(c) (((char)c>31&&(char)c<127)||(char)c=='\n'||(char)c=='\t')
FILE *serial;
void init(){
	setenv("GSK_RENDERER","gl",1);
	return;
}
void closeSerial(){
	if(isSerialOpen)
		fclose(serial),isSerialOpen=0;
	return;
}
void error(){
	if(isTempWindowOpen)
		gtk_window_destroy(GTK_WINDOW(tempWindow)),isTempWindowOpen=0;
	if(port)
		free(port),port=0;
	closeSerial(),enable(0);
	return;
}
void setup(){
	char *cmd=dynastr_strappend(dynastr_strappend("stty -F ",port,0)," cs8 9600 ignbrk -brkint -icrnl -imaxbel -opost -onlcr -isig -icanon -iexten -echo -echoe -echok -echoctl -echoke noflsh -ixon -crtscts",0);
	system(cmd),free(cmd),serial=fopen(port,"r+");
	if(!serial){
		error();
		return;
	}
	setbuf(serial,0),fcntl(fileno(serial),F_SETFL,O_NONBLOCK),isSerialOpen=1,enable(1);
	return;
}
void writeSerial(char *str){
	fprintf(serial,"%s",str);
	return;
}
char *readSerial(){
	char *s=dynastr_strtmp("",0),c=getc(serial);
	for(unsigned t=0;1;c=getc(serial)){
		if(PRINTABLE(c))
			t=0,s=dynastr_strappend(s,DYNASTR_CHR2STR(c),2);
		else{
			t++;
			if(t>2000)
				break;
			usleep(1000);
		}
	}
	return s;
}
#endif
GtkApplication *app;
static void connect0(GtkWidget *widget,gpointer data){
	error(),port=dynastr_strtmp(gtk_editable_get_text(GTK_EDITABLE(entry)),0),setup();
	if(isSerialOpen)
		writeSerial("help"),free(readSerial());
	return;
}
static void closeTempWindow(GtkWidget *widget,gpointer data){
	enable(1);
	return;
}
void initTempWindow(char *name){
	enable(0),
	isTempWindowOpen=1,
	tempWindow=gtk_application_window_new(app),
	gtk_window_set_title(GTK_WINDOW(tempWindow),name),
	g_signal_connect(tempWindow,"destroy",G_CALLBACK(closeTempWindow),NULL);
	return;
}
static void attendeeList(GtkWidget *widget,gpointer data){
	initTempWindow("Attendee list");
	GtkWidget *scroll=gtk_scrolled_window_new(),*text=gtk_text_view_new();
	GtkTextBuffer *buffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW(text));
	writeSerial("attendee list");
	char *s=readSerial();
	s=dynastr_strntmp(s,strlen(s)-1,1),
	gtk_text_buffer_set_text(buffer,s,-1),
	free(s),
	gtk_text_view_set_editable(GTK_TEXT_VIEW(text),0),
	gtk_window_set_child(GTK_WINDOW(tempWindow),scroll),
	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll),text),
	gtk_window_present(GTK_WINDOW(tempWindow));
	return;
}
static void clearLog0(GtkDialog *dialog,gint responseID, gpointer data){
	if(responseID==GTK_RESPONSE_YES)
		writeSerial("clear log"),free(readSerial());
	gtk_window_destroy(GTK_WINDOW(dialog));
	return;
}
static void clearLog(GtkWidget *widget,gpointer data){
	GtkWidget *dialog=gtk_message_dialog_new(GTK_WINDOW(window),GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,GTK_MESSAGE_WARNING,GTK_BUTTONS_YES_NO,"Do you really want to delete the log file? All history would be lost!");
	g_signal_connect(dialog,"response",G_CALLBACK(clearLog0),NULL),
	gtk_widget_show(dialog);
	return;
}
static void dst(GtkWidget *widget,gpointer data){
	writeSerial("dst");
	return;
}
static void eraseFingerprints0(GtkDialog *dialog,gint responseID, gpointer data){
	if(responseID==GTK_RESPONSE_YES)
		writeSerial("erase fingerprints"),SLEEP(5000),free(readSerial());
	gtk_window_destroy(GTK_WINDOW(dialog));
	return;
}
static void eraseFingerprints(GtkWidget *widget,gpointer data){
	GtkWidget *dialog=gtk_message_dialog_new(GTK_WINDOW(window),GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,GTK_MESSAGE_WARNING,GTK_BUTTONS_YES_NO,"Do you really want to erase all fingerprints? You have to enroll new ones.\n(You will disconnect from the device after erasing is complete.)");
	g_signal_connect(dialog,"response",G_CALLBACK(eraseFingerprints0),NULL),
	gtk_widget_show(dialog);
	return;
}
static void format0(GtkDialog *dialog,gint responseID, gpointer data){
	if(responseID==GTK_RESPONSE_YES)
		writeSerial("format"),SLEEP(10000),free(readSerial()),error(),exit(0);
	gtk_window_destroy(GTK_WINDOW(dialog));
	return;
}
static void format(GtkWidget *widget,gpointer data){
	GtkWidget *dialog=gtk_message_dialog_new(GTK_WINDOW(window),GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,GTK_MESSAGE_WARNING,GTK_BUTTONS_YES_NO,"Do you really want to format storage? All data (except fingerprints' data) would be lost!\n(Also, the program will close after formatting. Please restart your device when formatting is complete.)");
	g_signal_connect(dialog,"response",G_CALLBACK(format0),NULL),
	gtk_widget_show(dialog);
	return;
}
static void resetData3(GtkDialog *dialog,gint responseID, gpointer data){
	if(responseID==GTK_RESPONSE_YES)
		writeSerial("reset data"),free(readSerial()),writeSerial("0"),free(readSerial());
	gtk_window_destroy(GTK_WINDOW(dialog));
	return;
}
static void resetData2(GtkWidget *widget,gpointer data){
	GtkWidget *dialog=gtk_message_dialog_new(GTK_WINDOW(tempWindow),GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,GTK_MESSAGE_WARNING,GTK_BUTTONS_YES_NO,"Do you really want to reset all users' data? Entrances & exits would reset.");
	g_signal_connect(dialog,"response",G_CALLBACK(resetData3),NULL),
	gtk_widget_show(dialog);
	return;
}
static void resetData1(GtkDialog *dialog,gint responseID, gpointer data){
	if(responseID==GTK_RESPONSE_YES){
		int n=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(data));
		char s[11];
		writeSerial("reset data"),free(readSerial()),sprintf(s,"%i",n),writeSerial(s),free(readSerial());
	}
	gtk_window_destroy(GTK_WINDOW(dialog));
	return;
}
static void resetData0(GtkWidget *widget,gpointer data){
	GtkWidget *dialog=gtk_message_dialog_new(GTK_WINDOW(tempWindow),GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,GTK_MESSAGE_WARNING,GTK_BUTTONS_YES_NO,"Do you really want to reset the specified user's data? Entrances & exits would reset.");
	g_signal_connect(dialog,"response",G_CALLBACK(resetData1),data),
	gtk_widget_show(dialog);
	return;
}
static void resetData(GtkWidget *widget,gpointer data){
	int n;
	{
		writeSerial("saved fingerprints");
		char *s=readSerial(),tmp[3][12];
		sscanf(s,"%s%s%s%i",tmp[0],tmp[1],tmp[2],&n),free(s);
	}
	if(!n){
		GtkWidget *dialog=gtk_message_dialog_new(GTK_WINDOW(window),GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,GTK_MESSAGE_INFO,GTK_BUTTONS_OK,"You can't use this feature because no fingerprints are available!");
		gtk_widget_show(dialog);
		return;
	}
	GtkWidget *grid=gtk_grid_new(),*label=gtk_label_new("Fingerprint ID"),*tempSpinbutton=gtk_spin_button_new(gtk_adjustment_new(1,1,n+1,1,1,1),1,0),*(tempButton[2])={gtk_button_new_with_label("Reset data"),gtk_button_new_with_label("Reset all data")};
	initTempWindow("Reset data"),
	gtk_widget_set_halign(grid,GTK_ALIGN_CENTER),
	gtk_widget_set_valign(grid,GTK_ALIGN_CENTER),
	g_signal_connect(tempButton[0],"clicked",G_CALLBACK(resetData0),tempSpinbutton),
	g_signal_connect(tempButton[1],"clicked",G_CALLBACK(resetData2),NULL),
	gtk_window_set_child(GTK_WINDOW(tempWindow),grid),
	gtk_grid_attach(GTK_GRID(grid),label,0,0,2,1),
	gtk_grid_attach(GTK_GRID(grid),tempSpinbutton,0,1,2,1),
	gtk_grid_attach(GTK_GRID(grid),tempButton[0],0,2,1,1),
	gtk_grid_attach(GTK_GRID(grid),tempButton[1],1,2,1,1),
	gtk_window_present(GTK_WINDOW(tempWindow));
	return;
}
static void savedFingerprints(GtkWidget *widget,gpointer data){
	initTempWindow("Saved fingeprints"),
	writeSerial("saved fingerprints");
	char *s=readSerial();
	s=dynastr_strntmp(s,strlen(s)-1,1);
	GtkWidget *box=gtk_box_new(GTK_ORIENTATION_VERTICAL,0),*text=gtk_label_new(s);
	free(s);
	{
		GtkCssProvider* provider=gtk_css_provider_new();
		gtk_css_provider_load_from_data(provider,"label{font-size:20px;}",-1),
		gtk_style_context_add_provider(gtk_widget_get_style_context(text),GTK_STYLE_PROVIDER(provider),GTK_STYLE_PROVIDER_PRIORITY_USER);
	}
	gtk_widget_set_halign(box,GTK_ALIGN_CENTER),
	gtk_widget_set_valign(box,GTK_ALIGN_CENTER),
	gtk_window_set_child(GTK_WINDOW(tempWindow),box),
	gtk_box_append(GTK_BOX(box),text),
	gtk_window_present(GTK_WINDOW(tempWindow));
	return;
}
static void showLog(GtkWidget *widget,gpointer data){
	initTempWindow("Log");
	GtkWidget *scroll=gtk_scrolled_window_new(),*text=gtk_text_view_new();
	GtkTextBuffer *buffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW(text));
	writeSerial("show log");
	char *s=readSerial();
	s=dynastr_strntmp(s,strlen(s)-1,1),
	gtk_text_buffer_set_text(buffer,s,-1),
	free(s),
	gtk_text_view_set_editable(GTK_TEXT_VIEW(text),0),
	gtk_window_set_child(GTK_WINDOW(tempWindow),scroll),
	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll),text),
	gtk_window_present(GTK_WINDOW(tempWindow));
	return;
}
static void setTime(GtkWidget *widget,gpointer data){
	writeSerial("time"),free(readSerial());
	char s[22];
	sprintf(s,"%u,%hhu,%hhu,%hhu,%hhu,%hhu",gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spinbutton[0])),gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spinbutton[1])),gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spinbutton[2])),gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spinbutton[3])),gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spinbutton[4])),gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spinbutton[5]))),
	writeSerial(s),free(readSerial());
	return;
}
static void setSystemTime(GtkWidget *widget,gpointer data){
	writeSerial("time"),free(readSerial());
	char s[22];
	time_t rawtime=time(0);
	struct tm *info=localtime(&rawtime);
	sprintf(s,"%u,%hhu,%hhu,%hhu,%hhu,%hhu",info->tm_year+1900,info->tm_mon+1,info->tm_mday,info->tm_hour,info->tm_min,info->tm_sec),
	writeSerial(s),free(readSerial());
	return;
}
static void changeTime(GtkWidget *widget,gpointer data){
	initTempWindow("Time");
	GtkWidget *grid=gtk_grid_new(),*(label[6])={
		gtk_label_new("Year (YYYY)"),
		gtk_label_new("Month"),
		gtk_label_new("Day"),
		gtk_label_new("Hour"),
		gtk_label_new("Minute"),
		gtk_label_new("Second")
	},*(tempButton[2])={
		gtk_button_new_with_label("Set time"),
		gtk_button_new_with_label("Set system time")
	};
	spinbutton[0]=gtk_spin_button_new(gtk_adjustment_new(1970,1970,1000000,1,1,1),1,0),
	spinbutton[1]=gtk_spin_button_new(gtk_adjustment_new(1,1,13,1,1,1),1,0),
	spinbutton[2]=gtk_spin_button_new(gtk_adjustment_new(1,1,32,1,1,1),1,0),
	spinbutton[3]=gtk_spin_button_new(gtk_adjustment_new(0,0,24,1,1,1),1,0),
	spinbutton[4]=gtk_spin_button_new(gtk_adjustment_new(0,0,60,1,1,1),1,0),
	spinbutton[5]=gtk_spin_button_new(gtk_adjustment_new(0,0,60,1,1,1),1,0),
	gtk_widget_set_halign(grid,GTK_ALIGN_CENTER),
	gtk_widget_set_valign(grid,GTK_ALIGN_CENTER),
	gtk_window_set_child(GTK_WINDOW(tempWindow),grid),
	g_signal_connect(tempButton[0],"clicked",G_CALLBACK(setTime),NULL),
	g_signal_connect(tempButton[1],"clicked",G_CALLBACK(setSystemTime),NULL),
	gtk_grid_attach(GTK_GRID(grid),label[0],0,0,1,1),
	gtk_grid_attach(GTK_GRID(grid),label[1],1,0,1,1),
	gtk_grid_attach(GTK_GRID(grid),label[2],2,0,1,1),
	gtk_grid_attach(GTK_GRID(grid),label[3],3,0,1,1),
	gtk_grid_attach(GTK_GRID(grid),label[4],4,0,1,1),
	gtk_grid_attach(GTK_GRID(grid),label[5],5,0,1,1),
	gtk_grid_attach(GTK_GRID(grid),spinbutton[0],0,1,1,1),
	gtk_grid_attach(GTK_GRID(grid),spinbutton[1],1,1,1,1),
	gtk_grid_attach(GTK_GRID(grid),spinbutton[2],2,1,1,1),
	gtk_grid_attach(GTK_GRID(grid),spinbutton[3],3,1,1,1),
	gtk_grid_attach(GTK_GRID(grid),spinbutton[4],4,1,1,1),
	gtk_grid_attach(GTK_GRID(grid),spinbutton[5],5,1,1,1),
	gtk_grid_attach(GTK_GRID(grid),tempButton[0],0,2,3,1),
	gtk_grid_attach(GTK_GRID(grid),tempButton[1],3,2,3,1),
	gtk_window_present(GTK_WINDOW(tempWindow));
	return;
}
static void attendanceTime0(GtkWidget *widget,gpointer data){
	int n=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(data));
	char s[11],*s0;
	writeSerial("attendance time"),free(readSerial()),sprintf(s,"%i",n),writeSerial(s),s0=readSerial(),s0=dynastr_strntmp(s0,strlen(s0)-1,1);
	GtkWidget *dialog=gtk_message_dialog_new(GTK_WINDOW(tempWindow),GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,GTK_MESSAGE_INFO,GTK_BUTTONS_NONE,"%s",s0);
	free(s0),gtk_widget_show(dialog);
	return;
}
static void attendanceTime(GtkWidget *widget,gpointer data){
	int n;
	{
		writeSerial("saved fingerprints");
		char *s=readSerial(),tmp[3][12];
		sscanf(s,"%s%s%s%i",tmp[0],tmp[1],tmp[2],&n),free(s);
	}
	if(!n){
		GtkWidget *dialog=gtk_message_dialog_new(GTK_WINDOW(window),GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,GTK_MESSAGE_INFO,GTK_BUTTONS_OK,"You can't use this feature because no fingerprints are available!");
		gtk_widget_show(dialog);
		return;
	}
	GtkWidget *box=gtk_box_new(GTK_ORIENTATION_VERTICAL,0),*label=gtk_label_new("Fingerprint ID"),*tempSpinbutton=gtk_spin_button_new(gtk_adjustment_new(1,1,n+1,1,1,1),1,0),*tempButton=gtk_button_new_with_label("Get data");
	initTempWindow("Attendance time"),
	gtk_widget_set_halign(box,GTK_ALIGN_CENTER),
	gtk_widget_set_valign(box,GTK_ALIGN_CENTER),
	g_signal_connect(tempButton,"clicked",G_CALLBACK(attendanceTime0),tempSpinbutton),
	gtk_window_set_child(GTK_WINDOW(tempWindow),box),
	gtk_box_append(GTK_BOX(box),label),
	gtk_box_append(GTK_BOX(box),tempSpinbutton),
	gtk_box_append(GTK_BOX(box),tempButton),
	gtk_window_present(GTK_WINDOW(tempWindow));
	return;
}
static void resetAttendance3(GtkDialog *dialog,gint responseID, gpointer data){
	if(responseID==GTK_RESPONSE_YES)
		writeSerial("reset attendance"),free(readSerial()),writeSerial("0"),free(readSerial());
	gtk_window_destroy(GTK_WINDOW(dialog));
	return;
}
static void resetAttendance2(GtkWidget *widget,gpointer data){
	GtkWidget *dialog=gtk_message_dialog_new(GTK_WINDOW(tempWindow),GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,GTK_MESSAGE_WARNING,GTK_BUTTONS_YES_NO,"Are you sure you want to reset all users' attendance time? The time will be reset.");
	g_signal_connect(dialog,"response",G_CALLBACK(resetAttendance3),NULL),
	gtk_widget_show(dialog);
	return;
}
static void resetAttendance1(GtkDialog *dialog,gint responseID, gpointer data){
	if(responseID==GTK_RESPONSE_YES){
		int n=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(data));
		char s[11];
		writeSerial("reset attendance"),free(readSerial()),sprintf(s,"%i",n),writeSerial(s),free(readSerial());
	}
	gtk_window_destroy(GTK_WINDOW(dialog));
	return;
}
static void resetAttendance0(GtkWidget *widget,gpointer data){
	GtkWidget *dialog=gtk_message_dialog_new(GTK_WINDOW(tempWindow),GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,GTK_MESSAGE_WARNING,GTK_BUTTONS_YES_NO,"Are you sure you want to reset the specified user's attendance time? The time will be reset.");
	g_signal_connect(dialog,"response",G_CALLBACK(resetAttendance1),data),
	gtk_widget_show(dialog);
	return;
}
static void resetAttendance(GtkWidget *widget,gpointer data){
	int n;
	{
		writeSerial("saved fingerprints");
		char *s=readSerial(),tmp[3][12];
		sscanf(s,"%s%s%s%i",tmp[0],tmp[1],tmp[2],&n),free(s);
	}
	if(!n){
		GtkWidget *dialog=gtk_message_dialog_new(GTK_WINDOW(window),GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,GTK_MESSAGE_INFO,GTK_BUTTONS_OK,"You can't use this feature because no fingerprints are available!");
		gtk_widget_show(dialog);
		return;
	}
	GtkWidget *grid=gtk_grid_new(),*label=gtk_label_new("Fingerprint ID"),*tempSpinbutton=gtk_spin_button_new(gtk_adjustment_new(1,1,n+1,1,1,1),1,0),*(tempButton[2])={gtk_button_new_with_label("Reset data"),gtk_button_new_with_label("Reset all data")};
	initTempWindow("Reset attendance time"),
	gtk_widget_set_halign(grid,GTK_ALIGN_CENTER),
	gtk_widget_set_valign(grid,GTK_ALIGN_CENTER),
	g_signal_connect(tempButton[0],"clicked",G_CALLBACK(resetAttendance0),tempSpinbutton),
	g_signal_connect(tempButton[1],"clicked",G_CALLBACK(resetAttendance2),NULL),
	gtk_window_set_child(GTK_WINDOW(tempWindow),grid),
	gtk_grid_attach(GTK_GRID(grid),label,0,0,2,1),
	gtk_grid_attach(GTK_GRID(grid),tempSpinbutton,0,1,2,1),
	gtk_grid_attach(GTK_GRID(grid),tempButton[0],0,2,1,1),
	gtk_grid_attach(GTK_GRID(grid),tempButton[1],1,2,1,1),
	gtk_window_present(GTK_WINDOW(tempWindow));
	return;
}
static void terminate(GtkWidget *widget,gpointer data){
	error();
	return;
}
static void activate(GtkApplication *app,gpointer userData){
	GtkWidget *grid=gtk_grid_new(),*space=gtk_label_new(0);
	window=gtk_application_window_new(app),
	gtk_window_set_title(GTK_WINDOW(window),"Fingerprint Attendance Machine GUI"),
	g_signal_connect(window,"destroy",G_CALLBACK(terminate),NULL),
	entry=gtk_entry_new(),
	gtk_entry_set_placeholder_text(GTK_ENTRY(entry),"Serial port"),
	g_signal_connect(entry,"activate",G_CALLBACK(connect0),NULL),
	button[0]=gtk_button_new_with_label("Connect to device"),
	button[1]=gtk_button_new_with_label("Show attendee list"),
	button[2]=gtk_button_new_with_label("Clear log"),
	button[3]=gtk_button_new_with_label("Toggle DST"),
	button[4]=gtk_button_new_with_label("Erase all fingerprints"),
	button[5]=gtk_button_new_with_label("Format storage"),
	button[6]=gtk_button_new_with_label("Reset data"),
	button[7]=gtk_button_new_with_label("Show saved fingerprints"),
	button[8]=gtk_button_new_with_label("Show log"),
	button[9]=gtk_button_new_with_label("Change time"),
	button[10]=gtk_button_new_with_label("Show attendance time"),
	button[11]=gtk_button_new_with_label("Reset attendance time"),
	g_signal_connect(button[0],"clicked",G_CALLBACK(connect0),NULL),
	g_signal_connect(button[1],"clicked",G_CALLBACK(attendeeList),NULL),
	g_signal_connect(button[2],"clicked",G_CALLBACK(clearLog),NULL),
	g_signal_connect(button[3],"clicked",G_CALLBACK(dst),NULL),
	g_signal_connect(button[4],"clicked",G_CALLBACK(eraseFingerprints),NULL),
	g_signal_connect(button[5],"clicked",G_CALLBACK(format),NULL),
	g_signal_connect(button[6],"clicked",G_CALLBACK(resetData),NULL),
	g_signal_connect(button[7],"clicked",G_CALLBACK(savedFingerprints),NULL),
	g_signal_connect(button[8],"clicked",G_CALLBACK(showLog),NULL),
	g_signal_connect(button[9],"clicked",G_CALLBACK(changeTime),NULL),
	g_signal_connect(button[10],"clicked",G_CALLBACK(attendanceTime),NULL),
	g_signal_connect(button[11],"clicked",G_CALLBACK(resetAttendance),NULL),
	enable(0),
	gtk_widget_set_halign(grid,GTK_ALIGN_CENTER),
	gtk_widget_set_valign(grid,GTK_ALIGN_CENTER),
	gtk_window_set_child(GTK_WINDOW(window),grid),
	gtk_grid_attach(GTK_GRID(grid),entry,0,0,3,1),
	gtk_grid_attach(GTK_GRID(grid),button[0],3,0,1,1),
	gtk_grid_attach(GTK_GRID(grid),space,0,1,4,1),
	gtk_grid_attach(GTK_GRID(grid),button[1],0,2,1,1),
	gtk_grid_attach(GTK_GRID(grid),button[2],1,2,1,1),
	gtk_grid_attach(GTK_GRID(grid),button[3],2,2,1,1),
	gtk_grid_attach(GTK_GRID(grid),button[4],0,3,1,1),
	gtk_grid_attach(GTK_GRID(grid),button[5],1,3,1,1),
	gtk_grid_attach(GTK_GRID(grid),button[6],2,3,1,1),
	gtk_grid_attach(GTK_GRID(grid),button[7],0,4,1,1),
	gtk_grid_attach(GTK_GRID(grid),button[8],1,4,1,1),
	gtk_grid_attach(GTK_GRID(grid),button[9],2,4,1,1),
	gtk_grid_attach(GTK_GRID(grid),button[10],3,2,1,1),
	gtk_grid_attach(GTK_GRID(grid),button[11],3,3,1,1),
	gtk_window_present(GTK_WINDOW(window));
	return;
}
int main(int argc,char **argv){
	init(),
	app=gtk_application_new("com.github.Amirreza-Ipchi-Haq.FingerprintAttendanceMachineGUI",G_APPLICATION_DEFAULT_FLAGS),
	g_signal_connect(app,"activate",G_CALLBACK(activate),NULL);
	int status=g_application_run(G_APPLICATION(app),argc,argv);
	g_object_unref(app);
	return status;
}
