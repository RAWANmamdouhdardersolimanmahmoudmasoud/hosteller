#include <iostream>
#include <stdlib.h>
#include <conio.h>
#include <cmath>
#include <cstring>
#include <fstream>
#include <sstream>

using namespace std;


struct node {

    string name;
    string rollNo;
    string mobileNo;
    string roomNo;

};

// A B-Tree node
class BTreeNode
{
    node *Tree;
    int t;
    BTreeNode **C;
    int n;
    bool leaf;
    ofstream fout;

public:

    BTreeNode(int temp, bool _leaf);

    void print();

    BTreeNode *member(string k);

    int findKey(string k);

    void insertNonFull(string k, string roll, string room, string mobile);

    void splitChild(int i, BTreeNode *y);

    void remove(string k);

    void removeFromLeaf(int idx);

    void removeFromNonLeaf(int idx);

    node getPred(int idx);

    node getSucc(int idx);

    void fill(int idx);

    void borrowFromPrev(int idx);

    void borrowFromNext(int idx);

    void merge(int idx);

    string formatRecord();

    void write();

    friend class BTree;
};

// A B-Tree
class BTree
{
    BTreeNode *root;
    int t;

public:

    BTree(int temp)
    {
        root = NULL;
        t = temp;
        read();
    }

    void print()
    {
        if (root != NULL) root->print();
    }

    BTreeNode* member(string k)
    {
        return (root == NULL)? NULL : root->member(k);
    }

    void insert(string k, string roll, string room, string mobile);

    void remove(string k);

    void write();

    void read();
};

void BTree::read() {
    fstream fin;

    fin.open("Hostel Records.csv", ios::in);

    string name, mobile, room, roll;
    string temp;

    getline(fin, temp);

    while(getline(fin, room, ',')) {
        getline(fin, mobile, ',');
        getline(fin, roll, ',');
        getline(fin, name);
        insert(name, roll, mobile, room);
    }
    fin.close();
}


// A constructor for B-Tree Node class
BTreeNode::BTreeNode(int t1, bool leaf1)
{

    t = t1;
    leaf = leaf1;

    Tree = new node[2*t-1];
    C = new BTreeNode *[2*t];

    n = 0;
}

// Function to search key k in subtree rooted with this node
BTreeNode *BTreeNode::member(string k) {
  int i = 0;
  while (i < n && k > Tree[i].roomNo)
    i++;

  if (Tree[i].roomNo.compare(k) == 0)
    return this;

  if (leaf == true)
    return NULL;

  return C[i]->member(k);
}

// A utility function that returns the index of the first key that is greater than or equal to k
int BTreeNode::findKey(string k)
{
    int idx=0;
    while (idx<n && Tree[idx].roomNo.compare(k) < 0)
        ++idx;
    return idx;
}

// A function to remove the key k from the sub-tree rooted with this node
void BTreeNode::remove(string k)
{
    int idx = findKey(k);

    if (idx < n && Tree[idx].roomNo.compare(k) == 0)
    {
        if (leaf)
            removeFromLeaf(idx);
        else
            removeFromNonLeaf(idx);
    }
    else
    {
        if (leaf)
        {
            cout << "\n\n\t\t\tThe Key "<< k <<" Does Not Exist in the Tree!\n";
            return;
        }

        bool flag = ( (idx==n)? true : false );

        if (C[idx]->n < t)
            fill(idx);

        if (flag && idx > n)
            C[idx-1]->remove(k);
        else
            C[idx]->remove(k);
    }
    return;
}

// A function to remove the idx-th key from this node - which is a leaf node
void BTreeNode::removeFromLeaf (int idx)
{

    for (int i=idx+1; i<n; ++i)
        Tree[i-1] = Tree[i];
    n--;
    return;
}

// A function to remove the idx-th key from this node - which is a non-leaf node
void BTreeNode::removeFromNonLeaf(int idx)
{

    node k = Tree[idx];

    if (C[idx]->n >= t)
    {
        node pred = getPred(idx);
        Tree[idx] = pred;
        C[idx]->remove(pred.roomNo);
    }

    else if  (C[idx+1]->n >= t)
    {
        node succ = getSucc(idx);
        Tree[idx] = succ;
        C[idx+1]->remove(succ.roomNo);
    }

    else
    {
        merge(idx);
        C[idx]->remove(k.roomNo);
    }
    return;
}
// A function to get predecessor of keys[idx]
node BTreeNode::getPred(int idx)
{

    BTreeNode *cur=C[idx];
    while (!cur->leaf)
        cur = cur->C[cur->n];

    return cur->Tree[cur->n-1];
}

// A function to get successor of keys[idx]
node BTreeNode::getSucc(int idx)
{

    BTreeNode *cur = C[idx+1];
    while (!cur->leaf)
        cur = cur->C[0];

    return cur->Tree[0];
}

// A function to fill child C[idx] which has less than t-1 keys
void BTreeNode::fill(int idx)
{

    if (idx!=0 && C[idx-1]->n>=t)
        borrowFromPrev(idx);

    else if (idx!=n && C[idx+1]->n>=t)
        borrowFromNext(idx);

    else
    {
        if (idx != n)
            merge(idx);
        else
            merge(idx-1);
    }
    return;
}

// A function to borrow a key from C[idx-1] and insert it into C[idx]
void BTreeNode::borrowFromPrev(int idx)
{

    BTreeNode *child=C[idx];
    BTreeNode *sibling=C[idx-1];

    for (int i=child->n-1; i>=0; --i)
        child->Tree[i+1] = child->Tree[i];

    if (!child->leaf)
    {
        for(int i=child->n; i>=0; --i)
            child->C[i+1] = child->C[i];
    }

    child->Tree[0] = Tree[idx-1];

    if(!child->leaf)
        child->C[0] = sibling->C[sibling->n];

    Tree[idx-1] = sibling->Tree[sibling->n-1];

    child->n += 1;
    sibling->n -= 1;

    return;
}

// A function to borrow a key from the C[idx+1] and place it in C[idx]
void BTreeNode::borrowFromNext(int idx)
{

    BTreeNode *child=C[idx];
    BTreeNode *sibling=C[idx+1];

    child->Tree[(child->n)] = Tree[idx];

    if (!(child->leaf))
        child->C[(child->n)+1] = sibling->C[0];

    Tree[idx] = sibling->Tree[0];

    for (int i=1; i<sibling->n; ++i)
        sibling->Tree[i-1] = sibling->Tree[i];

    if (!sibling->leaf)
    {
        for(int i=1; i<=sibling->n; ++i)
            sibling->C[i-1] = sibling->C[i];
    }

    child->n += 1;
    sibling->n -= 1;

    return;
}

// A function to merge C[idx] with C[idx+1]; C[idx+1] is freed after merging
void BTreeNode::merge(int idx)
{
    BTreeNode *child = C[idx];
    BTreeNode *sibling = C[idx+1];

    child->Tree[t-1] = Tree[idx];

    for (int i=0; i<sibling->n; ++i)
        child->Tree[i+t] = sibling->Tree[i];

    if (!child->leaf)
    {
        for(int i=0; i<=sibling->n; ++i)
            child->C[i+t] = sibling->C[i];
    }

    for (int i=idx+1; i<n; ++i)
        Tree[i-1] = Tree[i];

    for (int i=idx+2; i<=n; ++i)
        C[i-1] = C[i];

    child->n += sibling->n+1;
    n--;

    delete(sibling);
    return;
}

void BTree::remove(string k)
{
    if (!root)
    {
        cout << "\n\n\t\t\tThe Tree is Empty!\n\n";
        return;
    }

    root->remove(k);

    if (root->n==0)
    {
        BTreeNode *tmp = root;
        if (root->leaf)
            root = NULL;
        else
            root = root->C[0];

        delete tmp;
    }
    return;
}

// The main function that inserts a new key in this B-Tree
void BTree::insert(string k, string roll, string mobile, string room)
{

    if (root == NULL)
    {

        root = new BTreeNode(t, true);
        root->Tree[0].name = k;
        root->Tree[0].rollNo = roll;
        root->Tree[0].mobileNo = mobile;
        root->Tree[0].roomNo = room;
        root->n = 1;
    }
    else
    {

        if (root->n == 2*t-1)
        {

            BTreeNode *s = new BTreeNode(t, false);


            s->C[0] = root;

            s->splitChild(0, root);

            int i = 0;
            if (s->Tree[0].roomNo.compare(room) < 0)
                i++;
            s->C[i]->insertNonFull(k, roll, mobile, room);

            root = s;
        }
        else
            root->insertNonFull(k,roll, mobile, room);
    }
}

// A utility function to insert a new key in this node
// The assumption is, the node must be non-full when this function is called
void BTreeNode::insertNonFull(string k, string roll, string mobile, string room)
{

    int i = n-1;

    if (leaf == true)
    {

        while (i >= 0 && Tree[i].roomNo.compare(room) > 0)
        {
            Tree[i+1] = Tree[i];
            i--;
        }

        Tree[i+1].name = k;
        Tree[i+1].rollNo = roll;
        Tree[i+1].mobileNo = mobile;
        Tree[i+1].roomNo = room;
        n = n+1;
    }
    else
    {

        while (i >= 0 && Tree[i].roomNo.compare(room) > 0)
            i--;

        if (C[i+1]->n == 2*t-1)
        {

            splitChild(i+1, C[i+1]);

            if (Tree[i+1].roomNo.compare(room) < 0)
                i++;
        }
        C[i+1]->insertNonFull(k, roll, room, mobile);
    }
}

// A utility function to split the child y of this node
// Note that y must be full when this function is called
void BTreeNode::splitChild(int i, BTreeNode *y)
{

    BTreeNode *z = new BTreeNode(y->t, y->leaf);
    z->n = t - 1;

    for (int j = 0; j < t-1; j++)
        z->Tree[j] = y->Tree[j+t];

    if (y->leaf == false)
    {
        for (int j = 0; j < t; j++)
            z->C[j] = y->C[j+t];
    }

    y->n = t - 1;

    for (int j = n; j >= i+1; j--)
        C[j+1] = C[j];

    C[i+1] = z;

    for (int j = n-1; j >= i; j--)
        Tree[j+1] = Tree[j];

    Tree[i] = y->Tree[t-1];

    n = n + 1;
}

// Function to traverse all nodes in a subtree rooted with this node
void BTreeNode::print()
{

    int i;
    for (i = 0; i < n; i++)
    {

        if (leaf == false)
            C[i]->print();
        cout << "\n\t\t+-----------------------------------------------------+" << endl;
        cout << "\n\n\t\t\t\tName           :  " << Tree[i].name << endl;
        cout << "\n\t\t\t\tRoll No.       :  " << Tree[i].rollNo << endl;
        cout << "\n\t\t\t\tMobile No.     :  " << Tree[i].mobileNo << endl;
        cout << "\n\t\t\t\tRoom No.       :  " << Tree[i].roomNo << endl;
        cout << "\n\n\t\t+-----------------------------------------------------+\n" << endl;
    }

    if (leaf == false)
        C[i]->print();
}

// A Function to authorize Administrator login
int adminlogin(){

    int i=0;
    int pi=0;
    char ch;
    char username[10] = "admin";
    char us[10];
    char pwd[10];

    u:
    cout<<"\n\n\t\t\t+----------------------------------------+";
    cout<<"\n\n\t\t\t\tLogin Credentials";
    cout<<"\n\n\t\t\t+----------------------------------------+";
    cout<<"\n\n\t\t\tEnter User Name  : ";
    cin>>us;

    if(strcmp(username,us)==0){

        p:
        cout<<"\n\t\t\tEnter Password   : ";
        while((ch = getch())!=13){
            pwd[i] = ch;
            i++;
            cout<<"*";
        }
        pwd[i] = '\0';

        if(strcmp(pwd, "abc123") == 0){

            cout<<"\n\n\n\t\t\t\tLogin Successful!\n";
            system("cls");
            return 1;
        }

        else{
            cout<<"\n\n\t\t\tInvalid Password!\n";
            pi++;

            if(pi!=3){
                goto p;
            }

            else{
                cout<<"\n\n\n\t\t\tYou've Entered the Invalid Password more than 3 times.\n\n";
                return 0;
            }

        }
    }

    else{
        cout<<"\n\n\t\t\tInvalid User name!\n\n";
        goto u;
    }

    return 0;
}

// A Function to Display Room Types and their Rent
void rent(){

    cout<<"\n\n";
    cout<<"\n\n\n\t\t\t+---------------------------------------------------+";
    cout<<"\n\t\t\t\tRoom Type"<<"\t\tRent in Rs. (per Day)\n";
    cout<<"\n\t\t\t+---------------------------------------------------+";
    cout<<"\n\n\t\t\t\tCommon Bath\n";
    cout<<"\n\t\t\t\t1. Per Person"<<"\t\t\t400";
    cout<<"\n\t\t\t\t2. Twin - Sharing"<<"\t\t275";

    cout<<"\n\n\n\t\t\t*****************************************************\n";

    cout<<"\n\t\t\t\tAttached Bath\n";
    cout<<"\n\t\t\t\t1. Per Person"<<"\t\t\t750";
    cout<<"\n\t\t\t\t2. Twin - Sharing"<<"\t\t600";
    cout<<"\n\n\n\t\t\t+---------------------------------------------------+\n\n\n";


}

// A Function to display rules to be followed
void rules(){

    cout<<"\n\n     |---------------------------------------------------------------------------------------------------------|";
    cout<<"\n     |                                    Hostel Rules and Regulations                                         |\n";
    cout<<"     |---------------------------------------------------------------------------------------------------------|\n\n";
    cout<<"     |     Smoking, Alcohol & Narcotic consumption is strictly prohibited in and around the Hostel premises.   | \n";
    cout<<"     |     Strict action will be taken against offenders.                                                      |\n";
    cout<<"     |     The Management & Staff will not be responsible for personal belongings.                             |\n";
    cout<<"     |     Students must keep the Campus & Rooms clean.                                                        |\n";
    cout<<"     |     Defacing walls, equipment, furniture etc; is strictly prohibited.                                   |\n";
    cout<<"     |     Students must turn off all the electrical equipments & lights before leaving their rooms.           |\n";
    cout<<"     |     Any manner of festivities and noise making celebrations will not be entertained.                    |\n";
    cout<<"     |     Students during their stay in the hostel will be governed by the management rules.                  |\n";
    cout<<"\n     |---------------------------------------------------------------------------------------------------------|\n\n";
    cout<<"\n\n";

}

// Start of main function
int main()
{

    int c1,c2,c3;
    string name, roll, mobile, room;

    BTree t(3);  // min. keys 2

    while (true) {


        mainmenu:

        cout<<"\n\n\n\t\t\t+-----------------------------------------+";
        cout<<"\n\n\t\t\t           Hostel Management System";
        cout<<"\n\n\t\t\t+-----------------------------------------+\n";

        cout<<"\n\n\t\t\t                 1. Admin\n";
        cout<<"\t\t\t                 2. Student\n";
        cout<<"\t\t\t                 3. Exit\n";
        cout<<"\n\t\t\t+-----------------------------------------+\n\n";

        cout<<"\n\n\t\t\t             Enter an Option : ";
        cin>>c1;

        system("cls");

        if(c1 == 1){

            if(adminlogin()){

                while(1){

                    cout<<"\n\n\t\t\t+------------------------------------+";
                    cout<<"\n\n\t\t\t\t       Admin Login\n";
                    cout<<"\n\n\t\t\t+------------------------------------+\n\n";
                    cout<<"\n\t\t\t       1. Insert a Record";
                    cout<<"\n\t\t\t       2. Search Records";
                    cout<<"\n\t\t\t       3. Delete a Record";
                    cout<<"\n\t\t\t       4. View Database";
                    cout<<"\n\t\t\t       5. Back to Main Menu";
                    cout<<"\n\t\t\t+------------------------------------+";

                    cout<<"\n\n\t\t\t     Enter an Option: ";
                    cin>>c3;

                    if(c3 == 1){

                            system("cls");

                            cout<<"\n\n\t\t\t+------------------------------------+";
                            cout<< "\n\n\t\t\t\tInserting a Record\n";
                            cout<<"\n\n\t\t\t+------------------------------------+\n\n";
                            cout << "\n\n\t\t\tEnter Name          : ";
                            cin >> name;

                            cout << "\n\t\t\tEnter Roll Number   : ";
                            cin >> roll;

                            cout << "\n\t\t\tEnter Mobile Number : ";
                            cin >> mobile;

                            cout << "\n\t\t\tEnter Room Number   : ";
                            cin >> room;

                            t.insert(name, roll, mobile, room);


                    }

                    if(c3 == 2){

                            system("cls");


                            cout<<"\n\n\t\t\t+------------------------------------+";
                            cout<< "\n\n\t\t\t\t\tSearch\n";
                            cout<<"\n\n\t\t\t+------------------------------------+\n\n";
                            cout << "\n\n\t\t\tEnter Room No.     : ";
                            cin >> room;

                            if (t.member(room)) {
                                cout<<"\n\n\t\t\tRoom " << room << " is Not Vacant!\n";
                            }
                            else {
                                cout<<"\n\n\t\t\tRoom "  << room << " is Vacant.\n";
                            }
                        }

                    if(c3 == 3){

                            system("cls");

                            cout<<"\n\n\t\t\t+----------------------------------------+";
                            cout<< "\n\n\t\t\t\tDeleting a Record\n";
                            cout<<"\n\n\t\t\t+----------------------------------------+\n\n";
                            cout << "\n\n\t\t\tEnter the Room Number : ";
                            cin >> room;

                            t.remove(room);

                    }

                    if(c3 == 4){

                            system("cls");

                            cout<<"\n\n\t\t\t+----------------------------------------+";
                            cout << "\n\n\t\t\t\t\tDATABASE" << endl;
                            cout<<"\n\n\t\t\t+----------------------------------------+\n\n";
                            t.print();
                            cout << endl;

                    }

                    if(c3 == 5){

                        system("cls");

                        goto mainmenu;
                    }

                }

            }

            else{
                cout<<"\n\n\t\t\tLogin Unsuccessful!\n\n";
                exit(0);
            }

        }

        if(c1 == 2){

            while(true){


                cout<<"\n\n\t\t\t+---------------------------------------------+";
                cout<<"\n\n\t\t\t             Student Login";
                cout<<"\n\n\t\t\t+---------------------------------------------+\n\n";

                cout<<"\n\n\t\t\t         1. Check Room Availability\n";
                cout<<"\t\t\t         2. View Room Tariff\n";
                cout<<"\t\t\t         3. View Rules and Regulations\n";
                cout<<"\t\t\t         4. Back to Main Menu";
                cout<<"\n\n\t\t\t+---------------------------------------------+\n\n";

                cout<<"\n\n\t\t         Enter an Option: ";
                cin>>c2;

                if(c2 == 1){

                    string key;

                    cout<<"\n\t\t\tEnter the Room Number to be Searched: ";
                    cin>>key;

                    if(t.member(key)){
                        cout<<"\n\n\t\t\tRoom Not Vacant!"<<endl;
                    }

                    else{
                        cout<<"\n\t\t\tRoom is Vacant."<<endl;
                    }

                }

                if(c2 == 2){

                    system("cls");
                    rent();

                }

                if(c2 == 3){

                    system("cls");
                    rules();
                }

                if(c2 == 4){

                    system("cls");
                    goto mainmenu;
                }
            }
        }

        if(c1 == 3){
            break;
        }

    }

    return 0;
}
