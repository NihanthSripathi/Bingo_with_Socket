////////////////////////////////////////////////////////////////////////////////
// GAME DESCRIPTION                                                           //
////////////////////////////////////////////////////////////////////////////////
// This program implements a multiplayer Bingo game using socket programming.//
// Players connect via TCP sockets, take turns marking numbers on a 5x5 grid,//
// and compete to achieve a full row, column, or diagonal of marked numbers. //
// The game tracks win/loss history and supports two players.                //
///////////////////////////////////////////////////////////////////////////////

#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>

////////////////////////////////////////////////////////////////////////////////
// HEADER                                                                     //
////////////////////////////////////////////////////////////////////////////////
// Author: NIHANTH  , (Co - Author, MARGADARSHI, DEBUGGER) : HEMANTH          //
// Date: 18-12-2025                                                           //
// Description: Multiplayer Bingo Game with Socket Communication              //
////////////////////////////////////////////////////////////////////////////////

char PLAYER_2_IP_ADDRESS[] = "127.0.0.1";// CHANGE FOR CONTINUOUS PLAY WITH SAME PLAYER

////////////////////////////////////////////////////////////////////////////////
// MACROS FOR SOCKET PROGRAMMING                                              //
////////////////////////////////////////////////////////////////////////////////
#define PORT 8888              // Port number for socket communication
#define BUF_SIZE 100           // Buffer size for data transmission
#define PLAYERS_SIZE 2         // Number of players in the game
#define PLAYER_NO_1 0          // Index for Player 1
#define PLAYER_NO_2 1          // Index for Player 2
#define DEFAULT_STATUS 0       // Default status flag
#define SET_VALUE 1            // Set value flag
#define PLAYER_WIN 1           // Player win status
#define PLAYER_LOSE -1         // Player lose status
#define FETCH 1                // Fetch operation type
#define UPDATE 2               // Update operation type

////////////////////////////////////////////////////////////////////////////////
// MACROS FOR BINGO GAME                                                      //
////////////////////////////////////////////////////////////////////////////////
#define BINGO_CARD_SIZE 5      // Size of the Bingo card (5x5 grid)
#define ROW BINGO_CARD_SIZE    // Number of rows in the grid
#define COL BINGO_CARD_SIZE    // Number of columns in the grid

////////////////////////////////////////////////////////////////////////////////
// FUNCTION DECLARATIONS FOR BINGO GAME                                       //
////////////////////////////////////////////////////////////////////////////////
void display_grid(void);              // Displays the current Bingo grid
int  mark_number(int);                // Marks a number on the grid and checks for win
int  check_for_win(void);             // Checks if the player has won
int  is_number_duplicate(int);        // Checks if a number is already in the grid
int  generate_random_number(void);    // Generates a unique random number for the grid
void clear_terminal_lines(int);       // Clears specified number of lines in terminal
void display_loading_quote(void);     // Displays a loading quote with animation
void initialize_bingo_game();         // Initializes the Bingo game grid
int  send_to_bingo(int);              // Sends a number to the Bingo grid and checks win

////////////////////////////////////////////////////////////////////////////////
// FUNCTION DECLARATIONS FOR SOCKET PROGRAMMING                               //
////////////////////////////////////////////////////////////////////////////////
void handle_sigint(int);              // Handles SIGINT signal for graceful exit
int  setup_socket(int);               // Sets up socket connection for players
void update_game_status(int,int);     // Updates and fetches game history

////////////////////////////////////////////////////////////////////////////////
// GLOBAL VARIABLES FOR BINGO GAME                                           //
////////////////////////////////////////////////////////////////////////////////
int count = 0;                        // Counter for completed lines in Bingo
int bingo_grid[ROW][COL];             // 5x5 grid for Bingo numbers
int game_result = DEFAULT_STATUS;     // Result of the game (win/lose)

////////////////////////////////////////////////////////////////////////////////
// GLOBAL VARIABLES FOR SOCKET PROGRAMMING                                    //
////////////////////////////////////////////////////////////////////////////////
int current_player;                   // Current player number (1 or 2)
char player_names[PLAYERS_SIZE][20];  // Names of both players
int __name_transfer_flag = DEFAULT_STATUS; // Flag for name transfer between players
int player_1_fd, player_2_fd;         // File descriptors for player sockets
struct sockaddr_in server_address;    // Server address structure
socklen_t addrlen = sizeof(server_address); // Length of address structure
char communication_buffer[BUF_SIZE];                  // Buffer for data transmission

////////////////////////////////////////////////////////////////////////////////
// FUNCTION: handle_sigint                                                    //
////////////////////////////////////////////////////////////////////////////////
// Description: Handles the SIGINT signal (Ctrl+C) to gracefully close        //
//              socket connections and terminate the process.                 //
// Parameters: sig_no - Signal number (unused in this implementation)         //
// Returns: void                                                              //
////////////////////////////////////////////////////////////////////////////////
void handle_sigint(int sig_no){
    printf("Closing connection\n");
    close(player_1_fd);
    if(current_player == 1)close(player_2_fd);
    kill(getpid(),SIGTERM);
}
////////////////////////////////////////////////////////////////////////////////
// FUNCTION: main                                                             //
////////////////////////////////////////////////////////////////////////////////
// Description: Entry point of the program. Sets up signal handling, displays //
//              game history, prompts for player selection, establishes       //
//              socket connection, and manages the game loop for multiplayer  //
//              Bingo.                                                        //
// Parameters: void                                                           //
// Returns: int - Exit status (0 for success)                                 //
////////////////////////////////////////////////////////////////////////////////
int main(){
    signal(SIGINT,handle_sigint);
    update_game_status(game_result,FETCH);
    printf("Select Player No :\nPlayer - 1\nPlayer - 2\nEnter choice :");
    scanf("%d",&current_player);
    printf("Enter Player - %d's nick name :",current_player);
    scanf("%s",player_names[current_player-1]);
    strcpy(communication_buffer,player_names[current_player-1]);
    while(setup_socket(current_player))sleep(3);
    while(1){
        if (current_player == 1){
            if(!__name_transfer_flag){
                memset(player_names[PLAYER_NO_2], 0, sizeof(player_names[PLAYER_NO_2]));
                read(player_2_fd,player_names[PLAYER_NO_2],sizeof(player_names[PLAYER_NO_2])-1);
                write(player_2_fd,player_names[PLAYER_NO_1], strlen(player_names[PLAYER_NO_1]));
                __name_transfer_flag = SET_VALUE;
                continue;
            }
            printf("\nWating for player-2( %s )..\n",player_names[PLAYER_NO_2]);
            memset(communication_buffer, 0, sizeof(communication_buffer));


            ssize_t bytes_read = read(player_2_fd, communication_buffer, sizeof(communication_buffer) - 1);
            
            if(strncmp(communication_buffer,"-1",sizeof("-1"))==0 || (atoi(communication_buffer)==-1)){
                printf("\n( %s )You LOST the MATCH Better Luck Next Time..\n",player_names[PLAYER_NO_1]);
                game_result = PLAYER_LOSE;
                break;
            }
            if (bytes_read <= 0 || (strncmp(communication_buffer,"exit",sizeof("exit")) == 0)) {
                (bytes_read<0)?perror("Read failed"):printf("Player - 2 ( %s )disconnected.\n",player_names[PLAYER_NO_2]);
                break;
            }
            if(!send_to_bingo(atoi(communication_buffer))){
                memset(communication_buffer, 0, sizeof(communication_buffer));
                strcpy(communication_buffer,"-1");
                write(player_2_fd, communication_buffer, strlen(communication_buffer));
                printf("\n( %s )You WON the MATCH\n",player_names[PLAYER_NO_1]);
                game_result = PLAYER_WIN;
                break;
            }            

            printf("Player_2 ( %s ) choosed : %s\n\nType Your No : ",player_names[PLAYER_NO_2],communication_buffer);
            scanf("%s",communication_buffer);
            if(!send_to_bingo(atoi(communication_buffer))){
                memset(communication_buffer, 0, sizeof(communication_buffer));
                strcpy(communication_buffer,"-1");
                write(player_2_fd, communication_buffer, strlen(communication_buffer));
                printf("\n( %s )You WON the MATCH\n",player_names[PLAYER_NO_1]); 
                game_result = PLAYER_WIN;            
                break;
            }
            ssize_t bytes_sent = write(player_2_fd, communication_buffer, strlen(communication_buffer));
            if (bytes_sent < 0) {
                perror("Write failed");
                break;
            }
            if(strncmp(communication_buffer,"exit",4) == 0)break;

        }else if(current_player == 2){
            if(!__name_transfer_flag){
                write(player_1_fd,player_names[PLAYER_NO_2], strlen(player_names[PLAYER_NO_2]));
                memset(player_names[PLAYER_NO_1], 0, sizeof(player_names[PLAYER_NO_1]));
                read(player_1_fd,player_names[PLAYER_NO_1],sizeof(player_names[PLAYER_NO_1])-1);
                __name_transfer_flag = SET_VALUE;
                continue;
            }
            printf("Player_1 ( %s ) choosed : %s\n\nType Your No : ",player_names[PLAYER_NO_1],(!atoi(communication_buffer))?"\b\bYou to start":communication_buffer);
            scanf("%s",communication_buffer);
            if(!send_to_bingo(atoi(communication_buffer))){
                memset(communication_buffer, 0, sizeof(communication_buffer));
                strcpy(communication_buffer,"-1");
                write(player_1_fd, communication_buffer, strlen(communication_buffer));
                printf("\n( %s )You WON the MATCH\n",player_names[PLAYER_NO_2]);
                game_result = PLAYER_WIN;         
                break;
            }
            communication_buffer[strcspn(communication_buffer, "\n")] = 0;
            write(player_1_fd, communication_buffer, strlen(communication_buffer));

            if (strncmp(communication_buffer, "exit", 4) == 0)break;

            printf("\nWating for player-1( %s )..\n",player_names[PLAYER_NO_1]);
            memset(communication_buffer, 0, sizeof(communication_buffer));
            read(player_1_fd, communication_buffer, sizeof(communication_buffer) - 1);
            if(strncmp(communication_buffer,"-1",sizeof("-1"))==0 || (atoi(communication_buffer)==-1)){
                printf("\n( %s )You LOST the MATCH Better Luck Next Time..\n",player_names[PLAYER_NO_2]);
                game_result = PLAYER_LOSE;
                break;
            }
            if(!send_to_bingo(atoi(communication_buffer))){
                memset(communication_buffer, 0, sizeof(communication_buffer));
                strcpy(communication_buffer,"-1");
                write(player_1_fd, communication_buffer, strlen(communication_buffer));
                printf("\n( %s )You WON the MATCH\n",player_names[PLAYER_NO_2]);
                game_result = PLAYER_WIN;       
                break;
            }
            if(strncmp(communication_buffer, "exit", sizeof("exit")) == 0)break;
        }
    }
    update_game_status(game_result,UPDATE);
    update_game_status(game_result,FETCH);
    printf("Closing connection\n");
    close(player_1_fd);
    if(current_player == 1)close(player_2_fd);
    return 0;
}


////////////////////////////////////////////////////////////////////////////////
// FUNCTION: setup_socket                                                     //
////////////////////////////////////////////////////////////////////////////////
// Description: Sets up the socket connection for the specified player.      //
//              For Player 1, it acts as the server, binding to a port and   //
//              waiting for Player 2 to connect. For Player 2, it acts as the//
//              client, connecting to Player 1's IP address. Initializes the //
//              Bingo game after successful connection.                      //
// Parameters: current_player - The player number (1 or 2)                   //
// Returns: int - 0 on success, 1 on failure                                 //
////////////////////////////////////////////////////////////////////////////////
int setup_socket(int current_player){

    player_1_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (player_1_fd < 0) {
        perror("Socket failed");
        return 1;
    }

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT); 
    if(current_player == 1){        
        server_address.sin_addr.s_addr = INADDR_ANY;
        if (bind(player_1_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
            perror("Bind failed");
            close(player_1_fd);
            return 1;
        }
        if (listen(player_1_fd, 1) < 0) {
            perror("Listen failed");
            close(player_1_fd);
            return 1;
        }

        if(fork() == 0)execlp("ip","ip","-br","-4","addr",NULL);
        printf("\nShare IP inet address with Player-2\nTo Start Game\n");

        player_2_fd = accept(player_1_fd, (struct sockaddr *)&server_address, &addrlen);
        if (player_2_fd < 0) {
            perror("Accept failed");
            close(player_1_fd);
            return 1;
        }
        printf("Player - 2  connected!\n");
    }else if(current_player == 2){
        printf("Enter IP Of player - 1:\n Displayed on player-1's Display : ");
        if(sizeof(PLAYER_2_IP_ADDRESS)<=11)scanf("%s",PLAYER_2_IP_ADDRESS);

        if (inet_pton(AF_INET, PLAYER_2_IP_ADDRESS ,&server_address.sin_addr) <= 0) {
            perror("inet_pton failed");
            close(player_1_fd);
            setup_socket(current_player);
        }
        if (connect(player_1_fd, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
            printf("\rWaiting for Player-1 :");
            fflush(stdout);
            close(player_1_fd);
            return 1;
        }
        printf("Connected to Player - 1 \n");
    }
    initialize_bingo_game();
    return 0;
}
////////////////////////////////////////////////////////////////////////////////
// FUNCTION: update_game_status                                               //
////////////////////////////////////////////////////////////////////////////////
// Description: Updates or fetches the game history from files. Updates the   //
//              win/loss records in binary files, or displays the history in  //
//              a formatted table with counts.                                //
// Parameters: status - Game result (PLAYER_WIN or PLAYER_LOSE)               //
//             type - Operation type (UPDATE or FETCH)                        //
// Returns: void                                                              //
////////////////////////////////////////////////////////////////////////////////
void update_game_status(int status,int type){

    FILE *win_fp,*lose_fp;
    win_fp = fopen("/tmp/bingo_2_0_win_status.bin","ab+");
    lose_fp = fopen("/tmp/bingo_2_0_lose_status.bin","ab+");
    if((win_fp == NULL) || (lose_fp == NULL))perror("File Open Error\n");
    if(type == UPDATE)fprintf(status==PLAYER_WIN?win_fp:lose_fp,"\t%-25s %-20s %-20s\n",player_names[current_player-1],status==PLAYER_WIN?"WON  AGANIST":"LOST AGANIST",(current_player==1)?player_names[PLAYER_NO_2]:player_names[PLAYER_NO_1]);
    else if(type == FETCH){
        int win_count = 0, lose_count = 0;
        printf("\n");
        printf("╔══════════════════════════════════════════════════════════════════════════════╗\n");
        printf("║                                 GAME HISTORY                                 ║\n");
        printf("╠══════════════════════════════════════════════════════════════════════════════╣\n");
        printf("║                                 WIN RECORDS                                  ║\n");
        printf("╠══════════════════════════════════════════════════════════════════════════════╣\n");
        rewind(win_fp);
        while(fgets(communication_buffer,BUF_SIZE,win_fp)){
            communication_buffer[strcspn(communication_buffer, "\n")] = 0;
            printf(" %-76s \n", communication_buffer);
            win_count++;
        }
        if(win_count == 0) printf("║ %-76s ║\n", "No wins recorded yet.");
        printf("╠══════════════════════════════════════════════════════════════════════════════╣\n");
        printf("║                                 LOSE RECORDS                                 ║\n");
        printf("╠══════════════════════════════════════════════════════════════════════════════╣\n");
        rewind(lose_fp);
        while(fgets(communication_buffer,BUF_SIZE,lose_fp)){
            communication_buffer[strcspn(communication_buffer, "\n")] = 0;  // Remove newline
            printf(" %-76s \n", communication_buffer);
            lose_count++;
        }
        if(lose_count == 0) printf("║ %-76s ║\n", "No losses recorded yet.");
        printf("╚══════════════════════════════════════════════════════════════════════════════╝\n");
        printf("Total Wins: %d | Total Losses: %d | Total Matchs: %d\n\n", win_count, lose_count, win_count+lose_count);
    }
    fclose(win_fp);
    fclose(lose_fp);
}
////////////////////////////////////////////////////////////////////////////////
// FUNCTION: initialize_bingo_game                                            //
////////////////////////////////////////////////////////////////////////////////
// Description: Initializes the Bingo game by seeding the random number       //
//              generator, filling the Bingo grid with unique random numbers, //
//              clearing the terminal, and displaying the initial grid.       //
// Parameters: void                                                           //
// Returns: void                                                              //
////////////////////////////////////////////////////////////////////////////////
void initialize_bingo_game(){
    int random_number_address;
    display_loading_quote();
    srand(time(NULL)^getpid()^(unsigned long)&random_number_address);
    for(int i = 0 ; i< ROW ; i++){
        for(int j = 0 ; j< COL ; j++){
            bingo_grid[i][j] = generate_random_number();
        }
    }
    clear_terminal_lines(100);
    display_grid();
}
int send_to_bingo(int num){
    int WIN = mark_number(num);
        if(WIN) return 0;
    return 1;    
}

////////////////////////////////////////////////////////////////////////////////
// FUNCTION: generate_random_number                                           //
////////////////////////////////////////////////////////////////////////////////
// Description: Generates a unique random number between 1 and 25 that is not //
//              already present in the Bingo grid.                            //
// Parameters: void                                                           //
// Returns: int - A unique random number                                      //
////////////////////////////////////////////////////////////////////////////////
int generate_random_number(){
    int min = 1, max =25;
    int rand_num = 0;
    while(1){
        rand_num = (rand()%(max-min+1))+min;
        if(is_number_duplicate(rand_num))
            continue;
        else{
            return rand_num;
        }
    }

}
////////////////////////////////////////////////////////////////////////////////
// FUNCTION: is_number_duplicate                                              //
////////////////////////////////////////////////////////////////////////////////
// Description: Checks if a given number is already present in the Bingo grid //
// Parameters: val - The number to check                                      //
// Returns: int - 1 if duplicate, 0 otherwise                                 //
////////////////////////////////////////////////////////////////////////////////
int is_number_duplicate(int val){
    for(int i =0 ;i<ROW; i++){
        for(int j = 0; j<COL; j++)
            if(bingo_grid[i][j]==val)return 1;
    }
    return 0;
}
////////////////////////////////////////////////////////////////////////////////
// FUNCTION: display_grid                                                     //
////////////////////////////////////////////////////////////////////////////////
// Description: Prints the current state of the Bingo grid to the console.    //
// Parameters: void                                                           //
// Returns: void                                                              //
////////////////////////////////////////////////////////////////////////////////
void display_grid(){
    printf("\n");
    for(int i = 0 ; i< ROW ; i++){
        for(int j = 0 ; j< COL ; j++){
            printf(" %2d ",bingo_grid[i][j]);
        }printf("\n\n");
    }
    printf("\n");

}
////////////////////////////////////////////////////////////////////////////////
// FUNCTION: mark_number                                                      //
////////////////////////////////////////////////////////////////////////////////
// Description: Marks a number on the Bingo grid by setting it to 0, checks   //
//              for win conditions, clears terminal, and displays the grid.   //
// Parameters: val - The number to mark                                       //
// Returns: int - 1 if win, 0 otherwise                                       //
////////////////////////////////////////////////////////////////////////////////
int mark_number(int val){
    for(int i = 0 ; i< ROW ; i++){
        for(int j = 0 ; j< COL ; j++){
            if(bingo_grid[i][j] == val){
                bingo_grid[i][j] = 0;
            }
        }
    }
    count = check_for_win();
    clear_terminal_lines(100);
    if(count)printf("\n Congratulations Cleared : %d \n",count);
    display_grid();
    if(count >= ROW){
        clear_terminal_lines(100);
        printf("\n\n//***** BINGO *****\\\\\n\n"); 
        display_grid();
        printf("\\\\***** BINGO ****//\n\n\n"); 
        return 1;
    }else{
        return 0;
    }

}
////////////////////////////////////////////////////////////////////////////////
// FUNCTION: check_for_win                                                    //
////////////////////////////////////////////////////////////////////////////////
// Description: Checks the Bingo grid for completed lines (rows, columns,     //
//              diagonals) by counting how many lines are fully marked (all 0)//
// Parameters: void                                                           //
// Returns: int - Number of completed lines                                   //
////////////////////////////////////////////////////////////////////////////////
int check_for_win(){

    int count = 0;
    for(int i = 0 ; i< ROW ; i++){
        for(int j = 0 ; j< COL-1; j++){
            if(bingo_grid[i][j] == bingo_grid[i][j+1]){
                if(j==COL-2) ++count;
            }else break;
        }
    }
    for(int i = 0 ; i< COL ; i++){
        for(int j = 0 ; j< ROW-1; j++){
            if(bingo_grid[j][i] == bingo_grid[j+1][i]){
                if(j==ROW-2) ++count;
            }else break;
        }
    }
    for(int i = 0 ; i< COL-1; i++){
        if(bingo_grid[i][i] != bingo_grid[i+1][i+1]) break;
        if(i == COL-2) count++;
    }
    for(int i = 0 ,j = COL-1; i< COL-1; i++,j--){
        if(bingo_grid[i][j] != bingo_grid[i+1][j-1]) break;
        if(i == COL-2) count++;
    }
    return count;
}
////////////////////////////////////////////////////////////////////////////////
// FUNCTION: display_loading_quote                                            //
////////////////////////////////////////////////////////////////////////////////
// Description: Displays a loading quote with animation, followed by a        //
//              "Loading BINGO" message with dots.                            //
// Parameters: void                                                           //
// Returns: void                                                              //
////////////////////////////////////////////////////////////////////////////////
void display_loading_quote(void){
    char quote[1000]= "Eyes down, hearts up. The game begins now!";
    clear_terminal_lines(100);
    for(int i = 0 ; quote[i]; i++){
        printf("%c",quote[i]);
        fflush(stdout);
        usleep(1000*100);
    }
    printf("\n\nLoading BINGO");
    for(int i = 25 ; i>=0; i--){
        printf(".");
        fflush(stdout);
        usleep(1000*100);
    }
}
////////////////////////////////////////////////////////////////////////////////
// FUNCTION: clear_terminal_lines                                             //
////////////////////////////////////////////////////////////////////////////////
// Description: Clears a specified number of lines in the terminal using ANSI //
//              escape sequences.                                             //
// Parameters: l - Number of lines to clear                                   //
// Returns: void                                                              //
////////////////////////////////////////////////////////////////////////////////
void clear_terminal_lines(int l){
	for (int i = 0; i < l - 1; i++){
		printf("\033[2K");
		printf("\033[A");
	}
	printf("\r\033[2k");
	fflush(stdout);
}
