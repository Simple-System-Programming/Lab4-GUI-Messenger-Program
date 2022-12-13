#include <gtk/gtk.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#define SERV_IP "192.168.31.128" //서버 IP 주소
#define PORT 8989 //서버 포트 번호
#define BUFFER_SIZE 100 //송수신 버퍼 크기

typedef struct {
  GtkWidget *TextView;
  char recv_str[BUFFER_SIZE];
} MyWidgetGroup;

int g_sockfd;
GtkTextBuffer *Buffer;

void EntryActivate();
void *SockRecv();
gboolean WidgetShowSafe();

char name[100] = "";

int main(int argc, char *argv[]) {
  //---Socket
  g_sockfd=socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in address;
  address.sin_family=AF_INET;
  address.sin_addr.s_addr=inet_addr(SERV_IP);
  address.sin_port=htons(PORT);

  size_t len=sizeof(address);
  int result=connect(g_sockfd, (struct sockaddr*)&address, len);

  if(result == -1) {
    perror("서버 접속에 실패했습니다.");
    exit(1);
  }
  
  strcpy(name, strcat(argv[1]," : "));
  
  printf("%s",name);
  gtk_init (&argc, &argv);

  //---윈도우 설정
  GtkWidget *Window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size(GTK_WINDOW(Window), 320,240);
  gtk_window_set_title(GTK_WINDOW(Window), "MASSENGER PROGRAM");

  //---레이아웃 컨테이너 설정
  GtkWidget *Layout1=gtk_box_new(GTK_ORIENTATION_VERTICAL,0);

  //---여러줄 글상자 설정
  //GtkWidget *TextView=gtk_text_view_new();
  MyWidgetGroup *MyWidget=malloc(sizeof(MyWidgetGroup));
  MyWidget->TextView=gtk_text_view_new();
  Buffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW(MyWidget->TextView));
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(MyWidget->TextView), 1); 
  //gtk_widget_set_size_request(TextView, -1,350);
  gtk_text_view_set_editable(GTK_TEXT_VIEW(MyWidget->TextView), 0);

  //---여러줄 글상자 스크롤 설정
  GtkWidget *ScrollTextview=gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy 
    (GTK_SCROLLED_WINDOW(ScrollTextview), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC); 

  //---한줄 글상자 설정
  GtkWidget *Entry = gtk_entry_new();

  //---컨테이너 등록
  gtk_container_add(GTK_CONTAINER(Window), Layout1);
  gtk_container_add(GTK_CONTAINER(ScrollTextview), MyWidget->TextView);
  gtk_box_pack_start(GTK_BOX(Layout1),ScrollTextview,1,1,0);
  gtk_container_add(GTK_CONTAINER(Layout1), Entry);

  //--- 이벤트 시그널 등록
  g_signal_connect(Window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
  g_signal_connect(Entry, "activate", G_CALLBACK(EntryActivate), NULL);

  //---쓰레드 생성(소켓리시브)
  GThread *RecvThread=g_thread_new(NULL,(GThreadFunc)SockRecv,MyWidget);

  //---화면에 윈도우 출력
  gtk_widget_show_all(Window);
  gtk_main();
  return 0;
}

void EntryActivate(GtkEntry *Entry, GtkTextView *TextView) {

  char na[100];
  strcpy(na,name);

  const gchar *send_text= strcat(na,strcat(gtk_entry_get_text(GTK_ENTRY(Entry)),"\n"));


  write(g_sockfd, send_text, BUFFER_SIZE);
  gtk_entry_set_text(GTK_ENTRY(Entry), "");
}

void *SockRecv(MyWidgetGroup *MyWidget) {
  //GtkTextIter end_iter;
  //gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(Buffer), &end_iter);
  //GtkTextMark *mark = gtk_text_buffer_create_mark(GTK_TEXT_BUFFER(Buffer), NULL, &end_iter, 1);

  //char recv_str[BUFFER_SIZE];
  int tmp;
  while(1) {
    tmp=read(g_sockfd, MyWidget->recv_str, BUFFER_SIZE);
    if(tmp) {
      g_idle_add((GSourceFunc)WidgetShowSafe,MyWidget);
      //---수신된 버퍼를 여러줄 글상자에 출력
      //gtk_text_buffer_insert(GTK_TEXT_BUFFER(Buffer), &end_iter, recv_str,-1);
      //---스크롤을 마지막 버퍼로 이동함
      //gtk_text_buffer_move_mark(GTK_TEXT_BUFFER(Buffer), mark, &end_iter);
      //gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(TextView), mark);
    }
  }
}

gboolean WidgetShowSafe(MyWidgetGroup *MyWidget) {
  GtkTextIter end_iter;
  gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(Buffer), &end_iter);
  GtkTextMark *mark = gtk_text_buffer_create_mark(GTK_TEXT_BUFFER(Buffer), NULL, &end_iter, 1);

  //---수신된 버퍼를 여러줄 글상자에 출력
  gtk_text_buffer_insert(GTK_TEXT_BUFFER(Buffer), &end_iter, MyWidget->recv_str,-1);
  //---스크롤을 마지막 버퍼로 이동함
  gtk_text_buffer_move_mark(GTK_TEXT_BUFFER(Buffer), mark, &end_iter);
  gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(MyWidget->TextView), mark);
}


