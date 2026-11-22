#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Players_data.h"

#define MAX_TEAMS 10
#define MAX_PLAYERS 50

typedef struct PlayerNode {
    int id;
    char name[51];
    char teamName[51];
    char role[20];
    int totalRuns;
    float battingAverage;
    float strikeRate;
    int wickets;
    float economyRate;
    float performanceIndex;
    struct PlayerNode* next;
} PlayerNode;

// tempnodes used for merging lists for displaying players for all temas for a role (for choice 5) so original nodes are not changed
typedef struct TempNode {
    PlayerNode* data;
    struct TempNode* next;
} TempNode;

typedef struct {
    int id;
    char name[51];
    int totalPlayers;
    float totalBatSR;
    int batCount;
    float avgBatSR;
    PlayerNode* roleLists[3]; // 0: Batsman, 1: Bowler, 2: All-rounder || sorted while inserting
} Team;

Team teamList[MAX_TEAMS];
const char* roleNames[] = {"Batsman", "Bowler", "All-rounder"};

int getRoleIndex(const char* role) {
    if (strcmp(role, "Batsman") == 0) return 0;
    if (strcmp(role, "Bowler") == 0) return 1;
    return 2;
}

Team* findTeamById(int id) {
    int low = 0, high = teamCount - 1;
    while (low <= high) {
        int mid = (low + high) / 2;
        if (teamList[mid].id == id) return &teamList[mid];
        if (teamList[mid].id < id) low = mid + 1;
        else high = mid - 1;
    }
    return NULL;
}

int playerIdExists(int id) {
    for (int i = 0; i < teamCount; i++) {
        for (int r = 0; r < 3; r++) {
            for (PlayerNode* p = teamList[i].roleLists[r]; p != NULL; p = p->next) {
                if (p->id == id) return 1;
            }
        }
    }
    return 0;
}

// inserts player in descending order of performance index.
void insertPlayer(Team* team, PlayerNode* p) {
    int r = getRoleIndex(p->role);
    PlayerNode* head = team->roleLists[r];
    if (head == NULL || p->performanceIndex > head->performanceIndex) {
        p->next = head;
        team->roleLists[r] = p;
    } else {
        PlayerNode* curr = head;
        while (curr->next != NULL && curr->next->performanceIndex >= p->performanceIndex) {
            curr = curr->next;
        }
        p->next = curr->next;
        curr->next = p;
    }
    team->totalPlayers++;
    // only batters and all-rounders count for batting strike rate
    if (r == 0 || r == 2) {
        team->totalBatSR += p->strikeRate;
        team->batCount++;
        team->avgBatSR = team->totalBatSR / team->batCount;
    }
}

PlayerNode* createPlayer(int id, const char* name, const char* teamName, const char* role,
                         int runs, float avg, float sr, int wickets, float er) {
    int r = getRoleIndex(role);
    PlayerNode* p = (PlayerNode*)malloc(sizeof(PlayerNode));
    if (p == NULL) {
        printf("Memory allocation failed!\n");
        exit(1);
    }
    p->id = id;
    strcpy(p->name, name);
    strcpy(p->teamName, teamName);
    strcpy(p->role, role);
    p->totalRuns = runs;
    p->battingAverage = avg;
    p->strikeRate = sr;
    p->wickets = wickets;
    p->economyRate = er;
    if (r == 0) {
        p->performanceIndex = (avg * sr) / 100.0f;
    } else if (r == 1) {
        p->performanceIndex = (wickets * 2.0f) + (100.0f - er);
    } else {
        p->performanceIndex = ((avg * sr) / 100.0f) + (wickets * 2.0f);
    }
    p->next = NULL;
    return p;
}

void initializeData() {
    for (int i = 0; i < teamCount; i++) {
        teamList[i].id = i + 1;
        strcpy(teamList[i].name, teams[i]);
        teamList[i].totalPlayers = 0;
        teamList[i].totalBatSR = 0;
        teamList[i].batCount = 0;
        teamList[i].avgBatSR = 0;
        teamList[i].roleLists[0] = teamList[i].roleLists[1] = teamList[i].roleLists[2] = NULL;
    }
    for (int i = 0; i < playerCount; i++) {
        for (int t = 0; t < teamCount; t++) {
            if (strcmp(teamList[t].name, players[i].team) == 0) {
                PlayerNode* p = createPlayer(
                    players[i].id, players[i].name, players[i].team, players[i].role,
                    players[i].totalRuns, players[i].battingAverage, players[i].strikeRate,
                    players[i].wickets, players[i].economyRate
                );
                insertPlayer(&teamList[t], p);
                break;
            }
        }
    }
}

TempNode* cloneList(PlayerNode* head) {
    TempNode dummy;
    TempNode* tail = &dummy;
    dummy.next = NULL;
    while (head != NULL) {
        TempNode* node = (TempNode*)malloc(sizeof(TempNode));
        if (node == NULL) {
            printf("Memory allocation failed!\n");
            exit(1);
        }
        node->data = head;
        node->next = NULL;
        tail->next = node;
        tail = node;
        head = head->next;
    }
    return dummy.next;
}

TempNode* mergeTwoLists(TempNode* a, TempNode* b) {
    TempNode dummy;
    TempNode* tail = &dummy;
    dummy.next = NULL;
    while (a != NULL && b != NULL) {
        if (a->data->performanceIndex >= b->data->performanceIndex) {
            tail->next = a;
            a = a->next;
        } else {
            tail->next = b;
            b = b->next;
        }
        tail = tail->next;
    }
    tail->next = (a != NULL) ? a : b;
    return dummy.next;
}

// merge k lists using recursion.
TempNode* mergeKLists(TempNode* lists[], int left, int right) {
    if (left == right) return lists[left];
    int mid = (left + right) / 2;
    return mergeTwoLists(mergeKLists(lists, left, mid), mergeKLists(lists, mid + 1, right));
}

void addPlayerToTeam() {
    int teamId, id, roleChoice, runs, wickets;
    char name[51];
    float avg, sr, er;
    printf("Enter Team ID (1-%d): ", teamCount);
    if (scanf("%d", &teamId) != 1) {
        scanf("%*[^\n]");
        printf("Invalid input!\n");
        return;
    }
    Team* team = findTeamById(teamId);
    if (team == NULL) {
        printf("Invalid Team ID!\n");
        return;
    }
    if (team->totalPlayers >= MAX_PLAYERS) {
        printf("Team %s is already full (%d players max)!\n", team->name, MAX_PLAYERS);
        return;
    }
    printf("Player ID (1-1000): ");
    if (scanf("%d", &id) != 1 || id < 1 || id > 1000) {
        scanf("%*[^\n]");
        printf("Invalid Player ID! Must be between 1 and 1000.\n");
        return;
    }
    if (playerIdExists(id)) {
        printf("Error: Player ID %d already exists!\n", id);
        return;
    }
    printf("Name: ");
    if (scanf(" %50[^\n]", name) != 1) { 
        printf("Invalid name!\n");
        return;
    }
    printf("Role (1-Batsman, 2-Bowler, 3-All-rounder): ");
    if (scanf("%d", &roleChoice) != 1 || roleChoice < 1 || roleChoice > 3) {
        scanf("%*[^\n]");
        printf("Invalid role!\n");
        return;
    }
    printf("Total Runs: ");
    if (scanf("%d", &runs) != 1 || runs < 0) {
        scanf("%*[^\n]");
        printf("Invalid runs!\n");
        return;
    }
    printf("Batting Average: ");
    if (scanf("%f", &avg) != 1 || avg < 0) {
        scanf("%*[^\n]");
        printf("Invalid batting average!\n");
        return;
    }
    printf("Strike Rate: ");
    if (scanf("%f", &sr) != 1 || sr < 0) {
        scanf("%*[^\n]");
        printf("Invalid strike rate!\n");
        return;
    }
    printf("Wickets: ");
    if (scanf("%d", &wickets) != 1 || wickets < 0) {
        scanf("%*[^\n]");
        printf("Invalid wickets!\n");
        return;
    }
    printf("Economy Rate: ");
    if (scanf("%f", &er) != 1 || er < 0) {
        scanf("%*[^\n]");
        printf("Invalid economy rate!\n");
        return;
    }
    insertPlayer(team, createPlayer(id, name, team->name, roleNames[roleChoice - 1], runs, avg, sr, wickets, er));
    printf("Player added successfully to Team %s!\n", team->name);
}

void displayPlayersOfTeam() {
    int teamId;
    printf("Enter Team ID (1-%d): ", teamCount);
    if (scanf("%d", &teamId) != 1) {
        scanf("%*[^\n]");
        printf("Invalid input!\n");
        return;
    }
    Team* team = findTeamById(teamId);
    if (team == NULL) {
        printf("Invalid Team ID!\n");
        return;
    }
    printf("\nPlayers of Team %s:\n", team->name);
    printf("========================================================================================\n");
    printf("ID\tName\t\t\tRole\t\tRuns\tAvg\tSR\tWkts\tER\tPerf.Idx\n");
    printf("========================================================================================\n");
    for (int r = 0; r < 3; r++) {
        for (PlayerNode* p = team->roleLists[r]; p != NULL; p = p->next) {
            printf("%d\t%s\t\t%s\t\t%d\t%.1f\t%.1f\t%d\t%.1f\t%.2f\n",
                   p->id, p->name, p->role, p->totalRuns, p->battingAverage,
                   p->strikeRate, p->wickets, p->economyRate, p->performanceIndex);
        }
    }
    printf("========================================================================================\n");
    printf("Total Players: %d\nAverage Batting Strike Rate: %.2f\n", team->totalPlayers, team->avgBatSR);
}

void displayTeamsByAvgSR() {
    int printed[MAX_TEAMS] = {0};
    printf("\nTeams Sorted by Average Batting Strike Rate\n");
    printf("=========================================================\n");
    printf("ID\tTeam Name\t\tAvg Bat SR\tTotal Players\n");
    printf("=========================================================\n");
    for (int count = 0; count < teamCount; count++) {
        int maxIdx = -1;
        for (int i = 0; i < teamCount; i++) {
            if (!printed[i] && (maxIdx == -1 || teamList[i].avgBatSR > teamList[maxIdx].avgBatSR)) {
                maxIdx = i;
            }
        }
        printed[maxIdx] = 1;
        printf("%d\t%s\t\t%.2f\t\t%d\n",
               teamList[maxIdx].id, teamList[maxIdx].name,
               teamList[maxIdx].avgBatSR, teamList[maxIdx].totalPlayers);
    }
    printf("=========================================================\n");
}

void displayTopKPlayers() {
    int teamId, roleChoice, k;
    printf("Enter Team ID (1-%d): ", teamCount);
    if (scanf("%d", &teamId) != 1) {
        scanf("%*[^\n]");
        printf("Invalid input!\n");
        return;
    }
    Team* team = findTeamById(teamId);
    if (team == NULL) {
        printf("Invalid Team ID!\n");
        return;
    }
    printf("Enter Role (1-Batsman, 2-Bowler, 3-All-rounder): ");
    if (scanf("%d", &roleChoice) != 1 || roleChoice < 1 || roleChoice > 3) {
        scanf("%*[^\n]");
        printf("Invalid role!\n");
        return;
    }
    printf("Enter number of players (K): ");
    if (scanf("%d", &k) != 1 || k <= 0) {
        scanf("%*[^\n]");
        printf("Invalid number! K must be greater than 0.\n");
        return;
    }
    printf("\nTop %d %s of Team %s:\n", k, roleNames[roleChoice - 1], team->name);
    printf("========================================================================================\n");
    printf("ID\tName\t\t\tRole\t\tRuns\tAvg\tSR\tWkts\tER\tPerf.Idx\n");
    printf("========================================================================================\n");
    PlayerNode* p = team->roleLists[roleChoice - 1];
    for (int i = 0; i < k && p != NULL; i++, p = p->next) {
        printf("%d\t%s\t\t%s\t\t%d\t%.1f\t%.1f\t%d\t%.1f\t%.2f\n",
               p->id, p->name, p->role, p->totalRuns, p->battingAverage,
               p->strikeRate, p->wickets, p->economyRate, p->performanceIndex);
    }
    printf("========================================================================================\n");
}

void displayAllPlayersByRole() {
    int roleChoice;
    printf("Enter Role (1-Batsman, 2-Bowler, 3-All-rounder): ");
    if (scanf("%d", &roleChoice) != 1 || roleChoice < 1 || roleChoice > 3) {
        scanf("%*[^\n]");
        printf("Invalid role!\n");
        return;
    }
    TempNode* tempLists[MAX_TEAMS];
    for (int i = 0; i < teamCount; i++) {
        tempLists[i] = cloneList(teamList[i].roleLists[roleChoice - 1]);
    }
    TempNode* curr = mergeKLists(tempLists, 0, teamCount - 1);
    printf("\n%s of all teams:\n", roleNames[roleChoice - 1]);
    printf("====================================================================================================\n");
    printf("ID\tName\t\t\tTeam\t\tRole\t\tRuns\tAvg\tSR\tWkts\tER\tPerf.Idx\n");
    printf("====================================================================================================\n");
    while (curr != NULL) {
        PlayerNode* p = curr->data;
        printf("%d\t%s\t\t%s\t\t%s\t\t%d\t%.1f\t%.1f\t%d\t%.1f\t%.2f\n",
               p->id, p->name, p->teamName, p->role, p->totalRuns, p->battingAverage,
               p->strikeRate, p->wickets, p->economyRate, p->performanceIndex);
        TempNode* next = curr->next;
        free(curr);
        curr = next;
    }
    printf("====================================================================================================\n");
}

void freeAllMemory() {
    for (int i = 0; i < teamCount; i++) {
        for (int r = 0; r < 3; r++) {
            PlayerNode* curr = teamList[i].roleLists[r];
            while (curr != NULL) {
                PlayerNode* next = curr->next;
                free(curr);
                curr = next;
            }
        }
    }
}

int main() {
    initializeData();
    int choice;
    while (1) {
        printf("\n========================================================================================\n");
        printf("ICC ODI Player Performance Analyzer\n");
        printf("========================================================================================\n");
        printf("1. Add Player to Team\n");
        printf("2. Display Players of a Specific Team\n");
        printf("3. Display Teams by Average Batting Strike Rate\n");
        printf("4. Display Top K Players of a Specific Team by Role\n");
        printf("5. Display all Players of specific role Across All Teams by performance index\n");
        printf("6. Exit\n");
        printf("========================================================================================\n");
        printf("Enter your choice: ");
        if (scanf("%d", &choice) != 1) {
            scanf("%*[^\n]");
            printf("Invalid choice!\n");
            continue;
        }
        switch (choice) {
            case 1: addPlayerToTeam(); 
                    break;
            case 2: displayPlayersOfTeam(); 
                    break;
            case 3: displayTeamsByAvgSR(); 
                    break;
            case 4: displayTopKPlayers(); 
                    break;
            case 5: displayAllPlayersByRole(); 
                    break;
            case 6: freeAllMemory(); 
                    return 0;
            default: printf("Invalid choice!\n");
        }
    }
}