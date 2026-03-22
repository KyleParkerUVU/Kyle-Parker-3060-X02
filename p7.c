#include <stdio.h>
#include <stdlib.h>

#define MAX 100

// Function prototypes
int fcfs(int arr[], int n);
int sstf(int arr[], int n);
int look(int arr[], int n);
int clook(int arr[], int n);

// Absolute difference
int diff(int a, int b) {
    return abs(a - b);
}

// ---------------- FCFS ----------------
int fcfs(int arr[], int n) {
    int total = 0;

    for (int i = 0; i < n - 1; i++) {
        total += diff(arr[i], arr[i + 1]);
    }

    return total;
}

// ---------------- SSTF ----------------
int sstf(int arr[], int n) {
    int visited[MAX] = {0};
    int total = 0;
    int current = arr[0];

    visited[0] = 1;

    for (int i = 1; i < n; i++) {
        int minDist = 100000;
        int index = -1;

        for (int j = 1; j < n; j++) {
            if (!visited[j]) {
                int d = diff(current, arr[j]);
                if (d < minDist) {
                    minDist = d;
                    index = j;
                }
            }
        }

        visited[index] = 1;
        total += minDist;
        current = arr[index];
    }

    return total;
}

// ---------------- SORT ----------------
void sort(int arr[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = i + 1; j < n; j++) {
            if (arr[i] > arr[j]) {
                int temp = arr[i];
                arr[i] = arr[j];
                arr[j] = temp;
            }
        }
    }
}

// ---------------- LOOK ----------------
int look(int arr[], int n) {
    int temp[MAX];

    for (int i = 0; i < n; i++) {
        temp[i] = arr[i];
    }

    int start = temp[0];
    sort(temp, n);

    int pos = 0;
    for (int i = 0; i < n; i++) {
        if (temp[i] == start) {
            pos = i;
            break;
        }
    }

    int total = 0;
    int current = start;

    // Move right
    for (int i = pos + 1; i < n; i++) {
        total += diff(current, temp[i]);
        current = temp[i];
    }

    // Then move left
    for (int i = pos - 1; i >= 0; i--) {
        total += diff(current, temp[i]);
        current = temp[i];
    }

    return total;
}

// ---------------- C-LOOK ----------------
int clook(int arr[], int n) {
    int temp[MAX];

    for (int i = 0; i < n; i++) {
        temp[i] = arr[i];
    }

    int start = temp[0];
    sort(temp, n);

    int pos = 0;
    for (int i = 0; i < n; i++) {
        if (temp[i] == start) {
            pos = i;
            break;
        }
    }

    int total = 0;
    int current = start;

    // Move right
    for (int i = pos + 1; i < n; i++) {
        total += diff(current, temp[i]);
        current = temp[i];
    }

    // Jump to smallest
    if (pos > 0) {
        total += diff(current, temp[0]);
        current = temp[0];
    }

    // Continue up to start position
    for (int i = 1; i < pos; i++) {
        total += diff(current, temp[i]);
        current = temp[i];
    }

    return total;
}

// ---------------- MAIN ----------------
int main() {
    int arr[MAX];
    int n = 0;

    // Read input
    while (scanf("%d", &arr[n]) != EOF && n < MAX) {
        n++;
    }

    printf("Assignment 7: Block Access Algorithm\n");
    printf("By: Your Name\n\n");

    printf("FCFS Total Seek: %d\n", fcfs(arr, n));
    printf("SSTF Total Seek: %d\n", sstf(arr, n));
    printf("LOOK Total Seek: %d\n", look(arr, n));
    printf("C-LOOK Total Seek: %d\n", clook(arr, n));

    return 0;
}