/*
 * intervals.c
 *
 *  Created on: Mar 3, 2016
 *      Author: jatin
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

typedef struct Node_s
{
    int start;
    int end;
    struct Node_s *left;
    struct Node_s *right;
} Node;

Node *intervals;

static Node *allocNode(int start, int end)
{
   Node *node = malloc(sizeof(Node));
   node->start = start;
   node->end = end;
   node->left = node->right = NULL;
   return node;
}

static void freeNode(Node *node)
{
    free(node);
}

static void cleanupIntervals(Node *root)
{
    if (root == NULL)
        return;
    cleanupIntervals(root->left);
    cleanupIntervals(root->right);
    freeNode(root);
}

static void cleanup(Node **proot)
{
    Node *root = *proot;
    cleanupIntervals(root);
    *proot = NULL;
    printf("----Cleaned up intervals----\n\n");
}

static void traversePrint(Node *root)
{
    if (root == NULL)
        return;
    traversePrint(root->left);
    printf("[%d,%d]", root->start, root->end);
    traversePrint(root->right);
}
static void printIntervals(Node *root)
{
    printf("[");
    traversePrint(root);
    printf("]\n");
}

static Node* getPredecessor(Node * root, Node **pparent)
{
    Node *subtree = root->left;
    Node *parent = root;
    if (subtree == NULL)
        return NULL;
    while (subtree->right)
    {
        parent = subtree;
        subtree = subtree->right;
    }
    *pparent = parent;
    return subtree;
}

static Node* getSuccessor(Node *root, Node **pparent)
{
    Node *subtree = root->right;
    Node *parent = root;
    if (subtree == NULL)
        return NULL;
    while (subtree->left)
    {
        parent = subtree;
        subtree = subtree->left;
    }
    *pparent = parent;
    return subtree;
}

// Add the spillover to current node by extending the node.
// While extending check with predecessor and successor if the node can be
// merged.
static void addInterval(Node **proot, int start, int end)
{
    Node *root;
    if (start >= end)
        return;

    if (*proot == NULL)
    {
        *proot = allocNode(start, end);
        return;
    }

    root = *proot;
    if (start >= root->start && end <= root->end)
        return;

    // Check non-overlap and send to left or right
    if (start > root->end)
    {
        addInterval(&root->right, start, end);
        return;
    }

    if (end < root->start)
    {
        addInterval(&root->left, start, end);
        return;
    }

    // overlap case
    if (start < root->start)
    {
        Node *predParent;
        Node *pred = getPredecessor(root, &predParent);
        // Check the extent of pred
        while (pred)
        {
            if (pred->end < start)   // disjoint
                break;

            // predecessor is contiguous with root
            // delete it and merge it with root.
            if (predParent == root)
                predParent->left = pred->left;
            else
                predParent->right = pred->left;
            root->start = pred->start;
            freeNode(pred);
            // move on to the next predecessor
            pred = getPredecessor(root, &predParent);
        }
        // At this point, the previous lower interval is not contiguous with
        // the new addition. Just extend the addition if not already done.
        if (root->start > start)
            root->start = start;
    }

    // Spillover on the right side
    if (end > root->end)
    {
        Node *succParent;
        Node *succ = getSuccessor(root, &succParent);
        // Check the successor's reach.
        while (succ)
        {
            if (succ->start > end)   // disjoint
                break;

            // successor is contiguous with root
            // delete it and merge it with root.
            if (succParent == root)
                succParent->right = succ->right;
            else
                succParent->left = succ->right;
            root->end = succ->end;
            freeNode(succ);
            // move on to the next predecessor
            succ = getSuccessor(root, &succParent);
        }
        // At this point, the next higher interval is not contiguous with
        // the new addition. Just extend the addition if not already done.
        if (root->end < end)
            root->end = end;
    }
}


// check if range lies in root, send down the spillover removal to down the tree.
// split or delete root for rest of the removal.
static void removeInterval(Node **proot, int start, int end)
{
    Node *root;
    int leftStart, leftEnd, rightStart, rightEnd;
    if (start >= end)
        return;

    if (*proot == NULL)
        return;
    root = *proot;

    // Check overlap and send to left or right
    if (start > root->end)
    {
        removeInterval(&root->right, start, end);
        return;
    }

    if (end < root->start)
    {
        removeInterval(&root->left, start, end);
        return;
    }

    // overlap cases now:
    // First trim the overflows
    if (start < root->start)
    {
        removeInterval(&root->left, start, root->start);
        start = root->start;
    }

    if (end > root->end)
    {
        removeInterval(&root->right, root->end, end);
        end = root->end;
    }

    // Now the trimming lies inside the root.
    if (start > root->start)
    {
        leftStart = root->start;
        leftEnd = start;
    }
    else
        leftStart = leftEnd = root->start;

    if (end < root->end)
    {
        rightStart = end;
        rightEnd = root->end;
    }
    else
        rightStart = rightEnd = root->end;

    // At this point [leftEnd, rightStart) would be the one going out
    if (leftEnd < rightStart)        // non zero interval
    {
        // Lets remove the root node first.
        Node *parent;
        Node *next = getSuccessor(root, &parent);
        if (next)
        {
            root->end = next->end;
            root->start = next->start;
            if (parent == root)
                parent->right = next->right;
            else
                parent->left = next->right;
            freeNode(next);
        }
        else
        {
            *proot = root->left;
            freeNode(root);
            root = *proot;
        }

        // Now we may be left with 2 new nodes at max.
    }

    // Check on both sides for over-removal.

    addInterval(proot, leftStart, leftEnd);
    addInterval(proot, rightStart, rightEnd);
}

static void add(int start, int end)
{
    addInterval(&intervals, start, end);
    printIntervals(intervals);
}

static void rem(int start, int end)
{
    removeInterval(&intervals, start, end);
    printIntervals(intervals);
}

    static void testCase1(void)
{
    add(15, 16);
    add(7,8);
    add(3,4);
    add(11,12);
    add(9,10);
    add(13,14);
    add(1,2);
    add(21,22);
    add(27,28);
    add(5,6);
    add(3,10);
    add(11,22);
    add(0,26);
    add(29,45);
    add(26,55);
    cleanup(&intervals);
}

static void testCase2(void)
{
    rem(30,40);
    add(15, 16);
    add(7,8);
    add(3,4);
    add(11,12);
    add(9,10);
    add(13,14);
    add(1,2);
    rem(13,14);
    add(5,6);
    add(3,10);
    add(11,22);
    add(0,26);
    add(29,45);
    add(26,55);
    rem(2,7);
    rem(20,30);
    rem(40,70);
    rem(0,100);
    cleanup(&intervals);
}

static void testCase3(void)
{
    add(4,4);
    add(5,2);
    add(0,0);
    rem(3,5);
    add(3,5);
    add(3,5);
    rem(5,3);
    rem(3,5);
    rem(0,10);
    cleanup(&intervals);
}

static void testCaseInEmail(void)
{
    add(1,5);
    rem(2,3);
    add(6,8);
    rem(4,7);
    add(2,7);
    cleanup(&intervals);
}

static void testCaseYM(void)
{
    add(1, 5);

    rem(2, 3);

    add(6, 8);

    rem(4, 7);

    add(2, 7);

    add(1, 16);

    rem(1, 3);

    add(2, 4);

    rem(-32, -25);

    add(-9, 38);

    add(-55, 83);

    rem(4, 61);

    add(-29, -2);

    add(40, 72);

    add(-77, -52);

    add(2, 7);

    rem(9, 93);

    add(-29, 87);

    add(-28, 21);

    add(-47, 71);

    add(-97, -80);

    add(-64, -41);

    add(73, 80);

    add(16, 69);

    rem(-46, -5);

    rem(42, 82);

    add(-52, 59);

    rem(17, 51);

    add(2, 11);

    add(-78, -29);

    rem(-26, 82);

    add(-47, 0);

    add(-76, -5);

    add(-5, -2);

    add(51, 56);

    rem(-13, 1);

    rem(-32, -14);

    rem(-46, -29);

    rem(16, 68);

    add(-77, -22);

    rem(4, 90);

    rem(-71, -23);

    add(-61, 74);

    rem(45, 93);

    add(-40, -36);

    add(-95, -55);

    rem(-28, 24);

    add(27, 58);

    add(-11, 69);

    add(-39, 25);

    add(-90, 3);

    rem(47, 87);

    add(1, 90);

    add(-22, 0);

    rem(-63, -11);

    add(-93, 86);

    add(40, 80);

    add(3, 11);

    add(-69, -13);

    add(23, 81);

    add(-67, 18);

    add(7, 15);

    add(-66, -58);

    rem(17, 24);

    add(1, 98);

    rem(-97, -95);

    add(60, 83);

    rem(-99, -89);

    add(-11, 78);

    add(25, 93);

    add(-58, 68);

    add(-82, 60);

    add(-14, -12);

    add(-24, 20);

    rem(-2, 0);

    add(-9, 95);

    add(0, 2);

    rem(-60, 8);

    add(-34, 84);

    add(-40, 80);

    rem(-99, 100);

    add(-40, 46);

    add(-70, 28);

    rem(-93, -23);

    rem(-95, -90);

    add(19, 43);

    add(9, 41);

    rem(0, 5);

    add(-74, -12);

    rem(-8, 38);

    rem(14, 81);

    add(-38, -23);

    add(49, 55);

    rem(-6, 37);

    rem(-79, -55);

    add(-66, 11);

    add(33, 35);

    rem(9, 20);

    add(60, 89);

    rem(36, 42);

    add(13, 49);

    rem(3, 35);

    add(-85, -32);

    rem(-15, -3);

    add(44, 58);

    add(-28, 82);

    add(43, 99);

    rem(-50, -40);

    add(-17, 98);

    add(-91, 73);

    add(-59, -42);

    add(-86, -64);

    add(-67, 70);

    add(67, 80);

    add(-22, 32);

    add(-34, -11);

    add(-66, -11);

    add(2, 55);

    add(4, 19);

    add(-67, 32);

    add(28, 47);

    add(-58, -13);

    add(-1, 30);

    add(49, 66);

    add(38, 101);

    rem(5, 9);

    rem(-68, 90);

    add(-87, -38);

    add(0, 2);

    rem(-34, -19);

    rem(39, 43);

    rem(-77, -22);

    rem(14, 26);

    rem(0, 65);

    rem(-15, -7);

    add(7, 17);

    rem(-7, 71);

    rem(-84, -71);

    rem(26, 35);

    rem(-64, -50);

    add(-38, 49);

    add(-50, -48);

    rem(-85, -24);

    add(-51, -7);

    rem(25, 68);

    rem(-15, -4);

    add(-60, 19);

    add(13, 22);

    add(-28, 56);

    rem(5, 17);

    add(15, 31);

    add(31, 80);

    rem(-83, -48);

    rem(-82, 63);

    add(-39, 20);

    add(-76, -45);

    add(-46, -24);

    add(26, 54);

    add(26, 38);

    add(-89, 60);

    add(24, 57);

    add(37, 67);

    add(19, 47);

    add(-46, -44);

    add(-66, -44);

    add(26, 77);

    add(-64, 32);

    add(1, 14);

    add(-45, 77);

    add(4, 19);

    rem(-14, -8);

    add(3, 16);

    rem(2, 9);

    add(-32, 64);

    rem(12, 29);

    add(-46, 86);

    rem(3, 45);

    rem(8, 49);

    add(-58, -3);

    add(-101, -8);

    rem(-51, -33);

    add(-27, -14);

    rem(40, 46);

    add(-69, 26);

    add(66, 70);

    rem(-47, 16);

    add(22, 90);

    add(9, 46);

    rem(-97, 42);

    add(-61, 70);

    add(52, 56);

    rem(60, 79);

    add(39, 51);

    add(-11, -8);

    rem(-58, 2);

    add(-95, -56);

    rem(-92, 85);

    add(-32, -10);

    add(2, 6);

    rem(-52, 45);

    add(4, 26);

    add(-82, -54);

    add(-98, 71);

    rem(-83, -7);

    add(-63, -28);

    add(-61, 78);

    add(-14, 28);

    add(25, 37);

    rem(-52, -7);

    add(7, 11);

    add(-66, 50);

    add(-15, -9);

    add(41, 67);

    rem(-76, -57);

    add(-68, -55);

    add(-16, -11);

    rem(-74, 59);

    add(4, 13);

    rem(-37, -12);

    add(16, 55);

    rem(49, 74);

    add(-50, -36);

    rem(-81, 80);

    rem(-1, 14);

    add(-57, 35);

    add(-29, -6);

    add(74, 81);

    add(-66, -33);

    add(60, 72);

    rem(8, 58);

    add(1, 70);

    rem(19, 61);

    add(-37, -25);

    rem(71, 99);

    rem(26, 77);

    rem(-9, -2);

    rem(-34, 95);

    add(-17, 55);

    add(22, 55);

    add(-79, -70);

    rem(-91, 41);

    rem(-84, 58);

    add(-69, 97);

    add(-18, 39);

    rem(-47, 26);

    add(17, 27);

    rem(62, 66);

    add(-35, 24);

    add(1, 5);

    add(-15, 75);

    rem(93, 100);

    add(-79, -63);

    add(-16, 27);

    add(6, 20);

    add(-49, -38);

    add(11, 50);

    add(40, 63);

    rem(-15, -6);

    add(17, 51);

    rem(-41, -37);

    add(3, 77);

    add(-15, -12);

    add(-86, 59);

    rem(-41, 66);

    rem(-52, 63);

    add(11, 100);

    rem(-66, -17);

    add(-27, 74);

    add(14, 28);

    add(-20, -17);

    add(0, 40);

    rem(-6, 91);

    add(-3, -1);

    add(-51, 21);

    add(-28, 72);

    add(34, 56);

    add(-49, -31);

    add(-49, 5);

    add(8, 42);

    add(-32, -6);

    rem(-93, -3);

    rem(-38, -8);

    add(-82, -19);

    rem(6, 61);

    add(-88, 25);

    add(-10, -4);

    add(77, 96);

    add(-92, -39);

    add(-54, -30);

    add(-84, 95);

    add(-19, 14);

    add(-26, -15);

    add(3, 21);

    add(14, 32);

    add(-14, -2);

    add(3, 44);

    rem(-57, -14);

    rem(2, 11);

    rem(-41, -38);

    add(-87, -68);

    rem(8, 11);

    rem(23, 67);

    add(12, 57);

    add(-87, 18);

    add(69, 88);

    add(25, 29);

    rem(49, 75);

    rem(2, 7);

    rem(-19, -12);

    rem(20, 76);

    add(-65, -61);

    add(4, 14);

    rem(29, 40);

    add(5, 23);

    add(6, 50);

    rem(8, 15);

    add(-68, 49);

    add(10, 14);

    rem(-29, 57);

    add(49, 89);

    add(52, 60);

    add(84, 100);

    rem(-74, -68);

    add(-82, 2);

    add(25, 88);

    add(1, 3);

    rem(44, 67);

    rem(-68, -2);

    add(-75, 5);

    add(-99, 93);

    add(-11, 70);

    add(1, 26);

    add(6, 96);

    rem(-25, -6);

    add(-41, -34);

    add(33, 38);

    add(45, 47);

    rem(8, 38);

    add(32, 76);

    rem(-32, 68);

    add(-1, 55);

    rem(-28, -1);

    rem(-19, 39);

    rem(-16, -13);

    rem(-41, 33);

    rem(-91, 26);

    add(28, 42);

    add(-18, 15);

    rem(0, 22);

    rem(-58, -29);

    add(1, 3);

    add(-4, 48);

    add(-68, -42);

    add(-97, 54);

    add(-33, -15);

    add(35, 46);

    add(-67, 65);

    rem(1, 21);

    add(-50, 86);

    add(-54, 50);

    rem(-25, -22);

    add(-3, 43);

    add(-92, -7);

    add(-11, 68);

    add(2, 41);

    rem(41, 54);

    add(50, 91);

    add(-59, -7);

    rem(19, 33);

    add(-1, 45);

    add(35, 44);

    rem(-54, -27);

    add(-79, -2);

    add(-60, -56);

    add(1, 33);

    add(-48, -15);

    rem(-95, -22);

    add(-78, -58);

    add(-61, -54);

    rem(-52, 43);

    add(-19, 91);

    rem(-4, -1);

    add(-68, 2);

    rem(-84, -9);

    add(41, 96);

    add(14, 88);

    add(0, 26);

    add(12, 101);

    rem(-39, -14);

    add(-14, -2);

    add(17, 46);

    add(-68, -35);

    rem(-57, 71);

    add(-52, -24);

    rem(-17, 39);

    add(24, 58);

    add(59, 93);

    add(-93, 49);

    add(-57, 80);

    add(-35, 27);

    add(-25, 65);

    rem(-32, 45);

    add(19, 47);

    add(-60, 73);

    add(65, 74);

    add(-9, 30);

    rem(-81, -41);

    rem(26, 55);

    add(4, 7);

    add(-5, 36);

    rem(1, 6);

    add(-75, -22);

    rem(-27, 80);

    add(-42, 13);

    rem(-58, -43);

    add(3, 7);

    add(16, 99);

    add(-61, -59);

    add(58, 91);

    rem(7, 78);

    add(-95, -64);

    rem(-54, 9);

    add(-43, -33);

    add(-33, -21);

    add(-29, -19);

    add(0, 16);

    add(-39, 18);

    rem(2, 6);

    rem(6, 8);

    add(-3, 4);

    add(-20, 45);

    rem(-88, -74);

    add(64, 66);

    add(-64, -13);

    add(-57, -45);

    add(-5, -1);

    rem(20, 27);

    rem(51, 69);

    add(-72, 19);

    rem(-93, 13);

    add(-90, -29);

    rem(24, 30);

    add(39, 51);

    add(17, 52);

    add(-78, 9);

    rem(-46, -41);

    add(-72, -56);

    add(-68, 23);

    add(-93, -1);

    rem(-33, 0);

    add(57, 62);

    rem(21, 91);

    add(-15, 17);

    add(3, 5);

    rem(-58, 15);

    add(-9, 56);

    add(10, 18);

    add(-62, -12);

    add(2, 30);

    add(-25, -22);

    add(-70, -50);

    rem(18, 47);

    add(-67, 84);

    add(44, 50);

    add(-86, -23);

    add(-97, -30);

    add(-85, 73);

    rem(63, 91);

    add(4, 60);

    add(4, 101);

    rem(-55, 86);

    add(-28, 2);

    add(-96, -42);

    add(6, 18);

    rem(-63, -10);

    add(53, 89);

    add(-61, -12);

    add(-30, -11);

    add(-81, 80);

    add(5, 16);

    add(-69, 32);

    rem(58, 100);

    add(-81, 75);

    rem(23, 46);

    add(-28, -14);

    rem(-52, 23);

    add(9, 36);

    add(43, 48);

    rem(27, 89);

    rem(-84, -66);

    add(-39, 42);

    rem(17, 24);

    add(45, 67);

    add(-3, -1);

    rem(-4, -1);

    add(-92, -90);

    add(-76, -5);

    rem(22, 37);

    rem(-85, 69);

    add(-44, -14);

    rem(86, 88);

    rem(-89, -87);

    add(-92, 83);

    add(-59, 67);

    add(11, 13);

    add(-83, 96);

    rem(-25, 76);

    add(-52, 60);

    rem(-45, 54);

    rem(-60, -17);

    add(-83, 52);

    add(-27, -5);

    add(-37, -28);

    add(-22, -1);

    add(43, 70);

    rem(2, 25);

    rem(-62, 13);

    rem(-71, 28);

    add(-28, 2);

    add(2, 54);

    add(-64, -15);

    add(-76, 0);

    add(18, 27);

    add(-75, 80);

    add(15, 83);

    rem(-71, -6);

    add(-5, -1);

    rem(-25, -20);

    rem(21, 57);

    add(-49, -5);

    rem(-43, 44);

    rem(-8, -3);

    add(47, 98);

    add(-34, 59);

    add(-12, -9);

    rem(-83, -76);

    rem(-25, -1);

    add(27, 59);

    rem(25, 31);

    add(24, 48);

    rem(37, 81);

    add(-29, -9);

    add(-95, 23);

    add(-69, -62);

    rem(45, 92);

    add(-57, 94);

    rem(-86, 9);

    rem(-2, 0);

    rem(-90, -4);

    add(-37, 12);

    add(20, 53);

    add(-27, 82);

    add(-80, 92);

    add(-100, -2);

    rem(-64, 49);

    add(-41, -31);

    rem(-35, 68);

    add(-74, -22);

    rem(17, 58);

    rem(-92, -32);

    add(2, 81);

    add(12, 55);

    rem(8, 35);

    rem(-27, -9);

    add(-5, 11);

    add(-15, 72);

    rem(-3, 43);

    rem(-9, 60);

    add(34, 52);

    rem(-18, 67);

    add(-60, -57);

    add(-17, 24);

    rem(-60, 52);

    rem(-87, 65);

    add(59, 66);

    add(-50, -36);

    rem(-83, 49);

    add(-43, 41);

    add(-2, 0);

    add(55, 91);

    add(0, 77);

    add(12, 72);

    add(87, 89);

    rem(-63, 46);

    rem(-98, -17);

    rem(-23, 82);

    rem(-84, -78);

    add(-27, 79);

    add(-86, 14);

    add(-76, -2);

    add(23, 29);

    rem(-7, -4);

    add(-69, -39);

    add(-52, -23);

    add(-11, -7);

    rem(5, 13);

    add(6, 12);

    add(15, 26);

    rem(-46, -15);

    add(-12, 14);

    add(7, 46);

    add(-39, 15);

    add(-24, 98);

    add(-97, 37);

    add(-70, 59);

    add(-79, 64);

    add(-11, -4);

    add(0, 82);
}

int main(void)
{
    testCase1();
    testCase2();
    testCase3();
    testCaseInEmail();
    testCaseYM();
    return 0;
}

