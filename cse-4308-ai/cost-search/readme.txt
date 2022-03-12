Name: Le Uyen Nguyen
Student ID: 100 171 8086
Programming language used: C
Omega compatible: Yes, I did run test on Omega
How the code is structured:
 - find_route.c contains all source code.
 - Code includes 13 functions (including main and one test function).
 - Uninformed and Informed Uninforme Cost Search share the same GraphSearch() function.
 - If heuristic file is provided, the program automatically runs Informed search. Else, Uninformed search.
How to run code (on Omega):
gcc find_route.c -o find_route
./find_route input.txt London Kassel
