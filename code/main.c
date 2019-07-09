#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "inc/btree.h"
#include "inc/jrb.h"
#include "inc/jval.h"
#include "inc/dllist.h"
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#define size 512

typedef struct
{
    GtkWidget *searchentry;
    GtkWidget *textview;
    GtkListStore *foundlist;
    GtkListStore *list;
} Widgets;

BTA *tree,*sug;
BTA *soundexTree = NULL;

GtkWidget  *window;
Widgets *widget;

GtkWidget  *restore, *about, *dev;
GtkWidget  *search;
GtkWidget  *addBtn, *editBtn, *delBtn , *undoBtn;
GtkWidget  *treeview;
GtkWidget  *aboutDialog;
GtkWidget  *message;

GtkTextBuffer *buff;
char message_txt[100];
char input[1000];
char s1[100];
int firstSoundexSug = 1;
char wordFind[50];
int isCheckSoundex = 0;

// Endcode word to Soundex code
static char code[128] = { 0 };

const char* soundex(const char *s) {
	static char out[5];
	int c, prev, i;
	out[0] = out[4] = 0;
	if (!s || !*s) return out;
	out[0] = *s++;
	prev = code[(int)out[0]];
	for (i = 1; *s && i < 4; s++) {
		if ((c = code[(int) * s]) == prev) continue;
		if (c == -1) prev = 0;	/* vowel as separator */
		else if (c > 0) {
			out[i++] = c + '0';
			prev = c;
		}
	}
	while (i < 4) out[i++] = '0';
	return out;
}

void add_code(const char *s, int c) {
	while (*s) {
		code[(int)*s] = code[0x20 ^ (int) * s] = c;
		s++;
	}
}

void init(){
	static const char *cls[] =
	{ "AEIOU", "", "BFPV", "CGJKQSXZ", "DT", "L", "MN", "R", 0};
	int i;
	for (i = 0; cls[i]; i++)
		add_code(cls[i], i - 1);
}
// End the soundex althogrithm

void cleanAllScreen(GtkTextBuffer *textbuffer, Widgets *app){
	gtk_entry_set_text(app->searchentry,"");
	gtk_text_buffer_set_text(textbuffer,"",-1);
}

void hideWidget(GtkWidget  *button){
	gtk_widget_hide(button);
}

void showWidget(GtkWidget  *button){
	gtk_widget_show(button);
}

void showMessage(char *str) {
	strcpy(message_txt, str);
	gtk_label_set_text(message, message_txt);
}

int check_prefix(char * str1,char * str2) {
	int slen2 = strlen(str2);
	int slen1 = strlen(str1);
	int i;
	if (slen1 < slen2)
		return 0;
	for (i = 0; i < slen2; i++)
		if (str1[i] != str2[i])
			return 0;
	return 1;
}


void xulyxau(char *input, char *output1){
	int i;
	for(i = 0; i < strlen(input); i++){
		if(input[i] == ' '){
			strcpy(input, input + i + 1);
			output1[i] = '\0';
			break;
		}
		output1[i] = input[i];
	}
}

void addToListSuggests(JRB nextword, int number) {
	GtkTreeIter Iter;
	JRB tmp;
	jrb_traverse(tmp, nextword) {
		gtk_list_store_append(widget->list, &Iter);
		gtk_list_store_set(widget->list, &Iter, 0, jval_s(tmp->key), -1 );
		if (number-- < 1)
			return;
	}
}

void findBySoundex(char *word, char *arr){
	soundexTree = btopn("data/soundexTree.dat",0,1);
	char soundexcode[5], name[50], soundex1[5];
	int i=0,rsize;
	BTint value;
	strcpy(soundexcode, soundex(word));
	btsel(soundexTree,"",soundex1, 5*sizeof(char), &rsize);

	while(btseln(soundexTree, name, soundex1, 5 * sizeof(char), &rsize) == 0 && i < 30){
		if(strcmp(soundexcode, soundex1) == 0 && strcmp( word, name) != 0) {
			strcat(arr, name);
        	strcat(arr, " ");
			i++;
		}
	}
	btcls(soundexTree);
}

void suggest(char * word, GdkEventKey keyEvent) {
	char temp[100],name[50],mean[1000],soundex[5];
	int i,j=0,rsize;
	int max;

	GtkTextBuffer *textbuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget->textview));
	GtkTreeIter Iter;
	JRB tmp, nextword;
	int check = 0;
	strcpy(temp, word);
	strcpy(wordFind, word);
	nextword = make_jrb();
	tree = btopn("data/english-vietnamese.dat", 0, 0);
	gtk_list_store_clear(widget->list);
	if (btsel(tree, word, mean, 500*sizeof(char), &rsize) ==  0) {
		check = 1;
		gtk_list_store_append(widget->list, &Iter); 
		gtk_list_store_set(widget->list, &Iter, 0, word, -1 );
	}
	if (check == 0)
		btins(tree, temp, "", sizeof(char));
	btsel(tree, word, mean, 500*sizeof(char), &rsize) ;
	for (i = 0; i < 100; i++) {
		if(btseln(tree, temp, mean, 500*sizeof(char), &rsize) == 0)
		{
			if (check_prefix(temp, word)) {
				jrb_insert_str(nextword, strdup(temp), JNULL);
			}
			else break;
		}
	}
	if(jrb_empty(nextword) && check == 0){
		isCheckSoundex = 1;
		showMessage("Có thể bạn đã gõ sai chính tả!\nẤn TAB để tự động chỉnh sửa từ!");
		gtk_text_buffer_set_text(textbuffer,"",-1);

	}else isCheckSoundex = 0;
	if (check == 0 && keyEvent.keyval == GDK_KEY_Tab) {
		if (!jrb_empty(nextword)){
			isCheckSoundex = 0;
			strcpy(temp, jval_s(jrb_first(nextword)->key));
			gtk_entry_set_text(GTK_ENTRY(widget->searchentry),temp);
			gtk_editable_set_position(GTK_EDITABLE(widget->searchentry), strlen(temp));
		}
	} else {
		addToListSuggests(nextword, 15);
		jrb_free_tree(nextword);	
	}
	if(check == 0) {
		btdel(tree, word);
	}	
	btcls(tree);
}

gboolean checkSoundex(Widgets *wg, GdkEvent  *event, gpointer   user_data){
	gchar *sugget, *entryget;
	char result[1000] = "";
	
	GtkTreeIter  iter;
	GdkEventKey key=event->key;
	entryget = gtk_entry_get_text(GTK_ENTRY(wg));

	if(key.keyval == GDK_KEY_Tab && isCheckSoundex == 1){
		if( firstSoundexSug == 1) {
			findBySoundex(entryget, result);
			strcpy(input,result);
			firstSoundexSug = 0;
		}
		if(strlen(input) > 0) {
			xulyxau(input, s1);
			if(strcmp(entryget, s1) != 0) {
				gtk_entry_set_text(wg, s1);
			}
			showMessage("Ấn TAB để chuyển từ tự động sửa tiếp theo!");
		}
		
		return 1;
	}
	return 0;
}

gboolean updating_suggest(GtkWidget * entry, GdkEvent * event, gpointer user_data) {
	GdkEventKey keyEvent = event->key;
	char word[50];
	int len;
	strcpy(word, gtk_entry_get_text(GTK_ENTRY(widget->searchentry)));
	if (keyEvent.keyval == GDK_KEY_Tab) {
		suggest(word, keyEvent);
	}
	else {
		if (keyEvent.keyval != GDK_KEY_BackSpace) {
			len = strlen(word);
			word[len] = keyEvent.keyval;
			word[len + 1] = '\0';
		}
		else {
			len = strlen(word);
			word[len - 1] = '\0';
		}
		for(int i = 0; i < strlen(word); i++){
        	if(word[i] >= 'A' && word[i] <= 'Z') word[i] += 32;
    	}
		suggest(word, keyEvent);
	}
	return FALSE;
}

void closeAboutBtn_click(GtkWidget *button, Widgets *app) {
	hideWidget(aboutDialog);
}

void RestoreData(){
	remove("data/english-vietnamese.dat");
    FILE* ori = fopen("data/english-vietnamese.old","rb");
    FILE* cur = fopen("data/english-vietnamese.dat", "wb");
    int n;
    char buffer[512];
    while (!feof(ori)) {
        n = fread(buffer, 1, size, ori);
        if (n == 0) break;
        fwrite(buffer, 1, n, cur);
    }
    fclose(ori);
    fclose(cur);   
}

void restoreBtn_click(GtkWidget *button, Widgets *app) {
	RestoreData();
	GtkTextBuffer *textbuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app->textview));
	cleanAllScreen(textbuffer, app);
	hideWidget(delBtn);
	hideWidget(editBtn);
	hideWidget(addBtn);
	hideWidget(undoBtn);
	showMessage("Dữ liệu gốc đã được khôi phục!");
}

void editBtn_click(GtkWidget *button, Widgets *app) {
	long value;
	GtkTextIter start, end;
	gchar *find_txt, *btext;
	GtkTextBuffer *textbuffer;

	tree = btopn("data/english-vietnamese.dat", 0, 0);
    textbuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app->textview));
    find_txt = gtk_entry_get_text(GTK_ENTRY(app->searchentry));
    gtk_text_buffer_get_bounds (textbuffer, &start, &end);	
    btext = gtk_text_buffer_get_text(textbuffer,&start,&end ,FALSE);

    if (bfndky(tree,find_txt,&value) == 0) {
        btupd(tree, find_txt, btext, strlen(btext) + 1); 
        showMessage("Đã cập nhật lại nghĩa của từ!");
    }
    hideWidget(addBtn);
    btcls(tree);
}

void addBtn_click(GtkWidget *button, Widgets *app) {
	long value;
	GtkTextIter start, end;
	gchar *find_txt, *btext;
	GtkTextBuffer *textbuffer;

	tree = btopn("data/english-vietnamese.dat", 0, 0);
    textbuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app->textview));
    find_txt = gtk_entry_get_text(GTK_ENTRY(app->searchentry));
    gtk_text_buffer_get_bounds (textbuffer, &start, &end);	
    btext = gtk_text_buffer_get_text(textbuffer,&start,&end ,FALSE);
    
    if (bfndky(tree,find_txt,&value) != 0) {
        btins(tree, find_txt, btext, strlen(btext) + 1); 
        showMessage("Đã thêm từ mới vào từ điển!");
    }
    hideWidget(addBtn);
	hideWidget(undoBtn);
    btcls(tree);
}

void delBtn_click(GtkWidget *button, Widgets *app){
    gchar *text;
    char defnFind[6500];
    int rsize;
    long value;
    GtkTextBuffer *textbuffer;

    tree = btopn("data/english-vietnamese.dat", 0, 0);
    textbuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app->textview)); 
    text = gtk_entry_get_text(GTK_ENTRY(app->searchentry));
    
    if (bfndky(tree,text,&value) == 0) {
        bdelky(tree, text);		
        cleanAllScreen(textbuffer, app);
        showMessage("Đã xóa từ!");
    }  
 	hideWidget(delBtn);
	hideWidget(editBtn);
	hideWidget(addBtn);
	hideWidget(undoBtn);
    btcls(tree); 
}

void enterToSearch(GtkButton *button, Widgets *app){	
    int check=1;
    char defnTemp[6500];			
    int rsize;
    gchar *find_txt;
    GtkTextBuffer *textbuffer;
    GtkTreeIter  iter;
    tree = btopn("data/english-vietnamese.dat", 0, 0);
    find_txt = gtk_entry_get_text(GTK_ENTRY(app->searchentry));
    textbuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app->textview));
    for(int i = 0; i < strlen(find_txt); i++){
        if(find_txt[i] >= 'A' && find_txt[i] <= 'Z') find_txt[i] += 32;
    }
	firstSoundexSug = 1;
    if (strcmp(find_txt,"") == 0){     
        cleanAllScreen(textbuffer, app);
		hideWidget(delBtn);
		hideWidget(editBtn);
		hideWidget(addBtn);
		hideWidget(undoBtn);
    } else {
        check = btsel(tree, find_txt, defnTemp, sizeof(defnTemp), &rsize);
        if (check){
            gtk_text_buffer_set_text(textbuffer,"Nhập nghĩa của từ mới này vào đây.\nSau đó ấn nút 'Thêm Từ' để thêm từ mới này vào từ điển.",-1);
	     	hideWidget(delBtn);
			hideWidget(editBtn);
			hideWidget(undoBtn);
			showWidget(addBtn);
            showMessage("Không tìm thấy từ này trong từ điển!\nBạn muốn thêm từ mới này vào từ điển?");
        }
        else {
        	showMessage("");
        	hideWidget(addBtn);
        	gtk_list_store_append(app->foundlist, &iter);
            gtk_list_store_set (app->foundlist, &iter, 0, find_txt, -1);
            gtk_text_buffer_set_text(textbuffer,defnTemp,-1);
			if(strcmp(defnTemp, textbuffer) != 0) showWidget(undoBtn);
            showWidget(delBtn);
            showWidget(editBtn);
        }
    }
    btcls(tree); 
}

void undoBtn_click(GtkButton *button, Widgets *app) {
    gchar *find_txt;
    GtkTextBuffer *textbuffer;
    tree = btopn("data/english-vietnamese.dat", 0, 0);
    find_txt = gtk_entry_get_text(GTK_ENTRY(app->searchentry));
    textbuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app->textview));
    char defnTemp[6500];
    int rsize;
    if( btsel(tree, find_txt, defnTemp, sizeof(defnTemp), &rsize) == 0){
        gtk_text_buffer_set_text(textbuffer,defnTemp,-1);
        showMessage("Đã quay lại dữ liệu trước đó!");
    }
    btcls(tree); 
}

main (int argc, char *argv[]) {
	btinit();
	init();

    GtkBuilder  *builder; 
    GError *error = NULL;
    widget = g_slice_new(Widgets);
    gtk_init (&argc, &argv);

    builder = gtk_builder_new ();
    gtk_builder_add_from_file (builder, "glade/dict.glade", NULL);
    window = GTK_WIDGET (gtk_builder_get_object (builder, "window"));
    aboutDialog = GTK_WIDGET (gtk_builder_get_object (builder, "aboutdialog"));
    restore = GTK_WIDGET (gtk_builder_get_object (builder, "restore"));
    about = GTK_WIDGET (gtk_builder_get_object (builder, "about"));
    search = GTK_WIDGET (gtk_builder_get_object (builder, "search"));
    addBtn = GTK_WIDGET (gtk_builder_get_object (builder, "add"));
    editBtn = GTK_WIDGET (gtk_builder_get_object (builder, "edit"));
    delBtn = GTK_WIDGET (gtk_builder_get_object (builder, "del"));
    undoBtn = GTK_WIDGET (gtk_builder_get_object (builder, "undo"));
    treeview = GTK_WIDGET (gtk_builder_get_object (builder, "treeview"));
    message = GTK_WIDGET (gtk_builder_get_object (builder, "message"));
    widget->searchentry = GTK_WIDGET( gtk_builder_get_object( builder, "searchentry" ));
    widget->textview = GTK_WIDGET( gtk_builder_get_object( builder, "textview" ));
    widget->foundlist = GTK_LIST_STORE( gtk_builder_get_object( builder, "foundlist"));

    GtkEntryCompletion *comple;
	comple = gtk_entry_completion_new();
    gtk_entry_completion_set_text_column(comple, 0);
    widget->list = gtk_list_store_new(1, G_TYPE_STRING);
    gtk_entry_completion_set_model(comple, GTK_TREE_MODEL(widget->list));
    gtk_entry_set_completion(GTK_ENTRY(widget->searchentry), comple);
    gtk_builder_connect_signals (builder, widget);
    g_object_unref (G_OBJECT (builder));
    gtk_widget_show (window);   
    gtk_main ();
   
    btcls(tree);   
    return 0;
}

