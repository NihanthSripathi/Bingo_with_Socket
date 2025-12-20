# Bingo 2.0

A multiplayer Bingo game implemented in C using socket programming for real-time communication between two players.

## Description

Bingo 2.0 is a classic Bingo game where two players compete to mark numbers on a 5x5 grid. Players take turns selecting numbers, and the first to complete a full row, column, or diagonal wins. The game uses TCP sockets for communication, with one player acting as the server and the other as the client. It also tracks and displays game history, including wins and losses.

## Features

- **Multiplayer Gameplay**: Supports two players over a network using socket programming.
- **Real-time Communication**: Players exchange moves instantly via TCP sockets.
- **Bingo Grid**: 5x5 grid filled with unique random numbers from 1 to 25.
- **Win Conditions**: Checks for completed rows, columns, and diagonals.
- **Game History**: Tracks and displays win/loss records in a formatted table.
- **Signal Handling**: Graceful exit on SIGINT (Ctrl+C).
- **Loading Animation**: Displays a quote and loading screen at startup.

## Prerequisites

- **Operating System**: Linux (uses system calls like `fork`, `execlp`, and socket functions).
- **Compiler**: GCC (GNU Compiler Collection).
- **Libraries**: Standard C libraries (included with GCC). No external dependencies required.
- **Network**: Both players must be on the same network for socket communication.

## Installation

1. Clone or download the repository.
2. Navigate to the project directory containing `bingo_2_0.c`.
3. The `.gitignore` file is included to exclude compiled binaries, temporary files, and OS-specific files from version control.

## Compilation

Compile the program using GCC:

```bash
gcc bingo_2_0.c -o bingo
```

This will generate an executable named `bingo`.

## How to Play

1. **Run the Game**:
   - On the first machine (Player 1 - Server):
     - Run `./bingo`
     - Select Player 1.
     - Enter a nickname.
     - The program will display the IP addresses (using `ip -br -4 addr`). Share the relevant IP with Player 2.
     - Wait for Player 2 to connect.

   - On the second machine (Player 2 - Client):
     - Run `./bingo`
     - Select Player 2.
     - Enter a nickname.
     - Enter the IP address of Player 1 when prompted (default is 127.0.0.1 for local testing).
     - Connect to Player 1.

2. **Gameplay**:
   - Player 1 starts first.
   - Players take turns entering a number from 1 to 25.
   - The number is marked (set to 0) on both grids if present.
   - The game checks for win conditions after each move.
   - The first player to complete 5 lines (rows, columns, or diagonals) wins.
   - Type "exit" to quit the game.

3. **History**:
   - Game results are saved to `/tmp/bingo_2_0_win_status.bin` and `/tmp/bingo_2_0_lose_status.bin`.
   - View history at the start and end of each game.

## Configuration

- **Port**: The game uses port 8888 by default. Change `PORT` in the code if needed.
- **IP Address**: For Player 2, the IP is hardcoded as "192.168.144.53". Update `PLAYER_2_IP_ADDRESS` in the code or enter it manually during runtime.
- **Grid Size**: The grid is 5x5. Modify `BINGO_CARD_SIZE` to change size (affects win conditions).

## Example Output

```
╔══════════════════════════════════════════════════════════════════════════════╗
║                                 GAME HISTORY                                 ║
╠══════════════════════════════════════════════════════════════════════════════╣
║                                 WIN RECORDS                                  ║
╠══════════════════════════════════════════════════════════════════════════════╣
║ Player1                    WON  AGANIST              Player2                 ║
╚══════════════════════════════════════════════════════════════════════════════╝
Total Wins: 1 | Total Losses: 0 | Total Matchs: 1

Select Player No :
Player - 1
Player - 2
Enter choice :1
Enter Player - 1's nick name :Player1
Share IP inet address with Player-2
To Start Game
Player - 2  connected!

  5  12  18  23  25

  3  14  19  24  1

  7  15  20  2  6

  9  16  21  4  8

 11  17  22  10 13

Type Your No : 5
Player_2 ( Player2 ) choosed : 12

Congratulations Cleared : 1

  0  12  18  23  25

  3  14  19  24  1

  7  15  20  2  6

  9  16  21  4  8

 11  17  22  10 13

//***** BINGO *****\\

//***** BINGO ****//

( Player1 )You WON the MATCH
```

## Troubleshooting

- **Connection Issues**: Ensure both machines are on the same network and firewalls allow connections on port 8888.
- **IP Address**: If the IP command fails, manually find the IP using `ip addr show` and share it.
- **File Permissions**: The program writes to `/tmp/`. Ensure write permissions.
- **Compilation Errors**: Make sure GCC is installed and all headers are available.

## Author

- **Name**: NIHANTH
- **Date**: 18-12-2025

## License

This project is open-source. Feel free to modify and distribute.

## Contributing

Contributions are welcome! Please fork the repository and submit a pull request with your changes.

## Acknowledgments

- Inspired by the classic Bingo game.
- Uses socket programming concepts from system programming.
