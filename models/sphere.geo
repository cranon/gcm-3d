Point(1) = {0, 0, 0, 1};
Point(2) = {-1, 0, 0, 1};
Point(3) = {0, 1, 0, 1};
Point(4) = {0, -1, 0, 1};
Point(5) = {0, 0, 1, 1};
Point(6) = {0, 0, -1, 1};
Point(7) = {1, 0, 0, 1};
Circle(1) = {6, 1, 2};
Circle(2) = {2, 1, 5};
Circle(3) = {5, 1, 7};
Circle(4) = {7, 1, 6};
Circle(5) = {6, 1, 3};
Circle(6) = {3, 1, 5};
Circle(7) = {5, 1, 4};
Circle(8) = {4, 1, 6};
Circle(9) = {2, 1, 4};
Circle(10) = {4, 1, 7};
Circle(11) = {7, 1, 3};
Circle(12) = {3, 1, 2};
Line Loop(14) = {12, 2, -6};
Ruled Surface(14) = {14};
Line Loop(16) = {6, 3, 11};
Ruled Surface(16) = {16};
Line Loop(18) = {11, -5, -4};
Ruled Surface(18) = {18};
Line Loop(20) = {12, -1, 5};
Ruled Surface(20) = {20};
Line Loop(22) = {1, 9, 8};
Ruled Surface(22) = {22};
Line Loop(24) = {9, -7, -2};
Ruled Surface(24) = {24};
Line Loop(26) = {10, -3, 7};
Ruled Surface(26) = {26};
Line Loop(28) = {4, -8, 10};
Ruled Surface(28) = {28};
Surface Loop(30) = {26, 28, 18, 16, 14, 20, 22, 24};
Volume(30) = {30};
