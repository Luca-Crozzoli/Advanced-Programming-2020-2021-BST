#include <algorithm>
#include <iostream>
#include <iterator> // std::forward_iterator_tag;
#include <memory>   //std::unique_pytr
#include <utility>  // std::move
#include <vector>   //std::vector used to create a vector of pair for the balance function
#include "ap_error.h"
using namespace std;

template <typename key_type, typename val_type, typename OP = std::less<key_type>>
class bst
{
    struct Node
    {
        std::pair<const key_type, val_type> pair_data;
        std::unique_ptr<Node> left;
        std::unique_ptr<Node> right;
        Node *parent;

        Node *get_parent() const noexcept { return parent; }
        Node *get_left() const noexcept { return left.get(); }
        Node *get_right() const noexcept { return right.get(); }

        // L-VALUE
        /*Constructor for Node called by insert when tree is empty*/
        explicit Node(const std::pair<const key_type, val_type> &x) : pair_data{x}, parent{nullptr} { std::cout << "l-value 1 arg\n"; }
        /*Constructor for Node called by insert when tree is no more empty, set in a proper way the parent pointer*/
        Node(const std::pair<const key_type, val_type> &x, Node *p) : pair_data{x}, parent{p} { std::cout << "l-value 2 arg\n"; }

        // R-VALUE
        /*Constructor for Node called by insert when tree is empty*/
        explicit Node(std::pair<const key_type, val_type> &&x) : pair_data{std::move(x)}, parent(nullptr) { std::cout << "r-value 1 arg" << std::endl; }
        /*Constructor for Node called by insert when tree is no more empty, set in a proper way the parent pointer*/
        Node(std::pair<const key_type, val_type> &&x, Node *p) : pair_data{std::move(x)}, parent{p} { std::cout << "r-value 2 arg" << std::endl; }

        /*Destructor of Node*/
        ~Node() noexcept { std::cout << "Node destructor" << std::endl; }
    };

    // COPY FUNCTION CALLED ITERATIVELY TO COPY ALL THE TREE
    /*copy function called iteratively to perfrom a deep copy of the tree when we use the copy constructor
     */
    void __copy(const std::unique_ptr<Node> &up)
    {
        if (up)
        { // if unique pointer != empty (it contains a raw pointer)
            insert(up->pair_data);
            __copy(up->left);
            __copy(up->right);
        }
    }

public:
    std::unique_ptr<Node> root_node;
    std::size_t tree_size;
    OP op;

    // DEFAULT CONSTRUCTOR AND DESTRUCTOR (default is enoguh because we dont' have raw pointers_______________________________________________________
    bst() = default;
    ~bst() noexcept = default;

    // TEMPLATED CLASS FOR THE ITERATOR, WE ARE INTERESTED IN O = data_pair const data_pair___________________________________________________________
    template <typename O>
    class _Iterator;

    using iterator = _Iterator<std::pair<const key_type, val_type>>;             // we pass the templated pair
    using const_iterator = _Iterator<const std::pair<const key_type, val_type>>; // we pass the const templated pair

    // BEGIN (for the range for loop which uses the iteratro) noexcept because we are not acquiring resources_____________________________________________
    iterator begin() noexcept
    {
        Node *tmp = root_node.get();
        while (tmp->left)
        {
            tmp = tmp->left.get();
        }
        return iterator{tmp};
    }
    const_iterator begin() const noexcept
    {
        Node *tmp = root_node.get();
        while (tmp->left)
        {
            tmp = tmp->left.get();
        }
        return const_iterator{tmp};
    }
    const_iterator cbegin() const noexcept { return begin(); }

    // END (for the range for loop which uses the iterator) noexcept because we are not acquairing resources__________________________________________________________________________
    iterator end() noexcept { return iterator{nullptr}; }
    const_iterator end() const noexcept { return const_iterator{nullptr}; }
    const_iterator cend() const noexcept { return const_iterator{nullptr}; };

    // FIND_________________________________________________________________________________________________________________________________________________________
    iterator find(const key_type &x) noexcept
    {
        for (iterator my_iterator{begin()}; my_iterator != end(); ++my_iterator)
        {
            if (!op(my_iterator->first, x) && !op(x, my_iterator->first))
            {
                return my_iterator;
            }
        }
        return end();
    };
    const_iterator find(const key_type &x) const noexcept
    {
        for (const_iterator my_iterator{begin()}; my_iterator != end(); ++my_iterator)
        {
            if (!op(my_iterator->first, x) && !op(x, my_iterator->first))
            {
                return my_iterator;
            }
        }
        return end();
    };

    // COPY AND MOVE SEMANTICS FOR THE BST____________________________________________________________________________________________________________________________
    // Move constructor (default because no raw pointers)
    bst(bst &&) noexcept = default;
    // move assignment (default because no raw pointers)
    bst &operator=(bst &&) noexcept = default;

    // Copy constructro for a deep copy
    bst(const bst &x) : tree_size{}
    {
        std::cout << "COPY CONSTRUCTOR" << std::endl;
        try
        {
            AP_ERROR(x.root_node) << "Nothing to copy the tree is empty";
        }
        catch (const exception &e)
        {
            std::cout << e.what() << std::endl;
        }
        __copy(x.root_node);
    }
    // Copy assignment, standard way
    bst &operator=(const bst &x)
    {
        std::cout << "COPY ASSIGNMENT" << std::endl;
        root_node.reset(); // remember to release the memory we own

        auto tmp = x;           // copy constructor
        *this = std::move(tmp); // move assignment
        return *this;
    }

    // INSERT___________________________________________________________________________________________________________________________________________________________________
    // here we define 2 insert functions calling the function _insert which uses the forwarding to avoid code duplication
    // to take only in one function r and l values
    template <typename P>
    std::pair<iterator, bool> _insert(P &&x)
    { // forwarding is not a n r-value
        std::cout << "forwarding _insert" << std::endl;
        auto tmp = root_node.get();
        while (tmp)
        {                                          // if there is a node in the tree we have to loop to decide in which point insert the new one
            if (op(x.first, tmp->pair_data.first)) // if x.key < tmp.key return true
            {
                if (!tmp->left)
                {                                                       // if tmp->left == nullptr
                    tmp->left.reset(new Node{std::forward<P>(x), tmp}); // we call the second constructor in which we set the parent pointer (l or r according o the type passed with forwarding)
                    ++tree_size;
                    iterator my_iterator{tmp->left.get()};
                    return std::pair<iterator, bool>{my_iterator, true}; // true because we insert a new node
                }
                else
                {
                    tmp = tmp->left.get();
                }
            }
            else if (op(tmp->pair_data.first, x.first)) // if x-key > tmp.key
            {
                if (!tmp->right) // if tmp-> right == nullptr
                {
                    tmp->right.reset(new Node{std::forward<P>(x), tmp}); // we call the second constructor in which we set the parent pointer (l or r according o the type passed with forwarding)
                    ++tree_size;
                    iterator my_iterator{tmp->right.get()};
                    return std::pair<iterator, bool>{my_iterator, true}; // true becasue we insert a new node
                }
                else
                {
                    tmp = tmp->right.get();
                }
            }
            else
            { // tmp.key == x.key
                iterator my_iterator = find(x.first);
                return std::pair<iterator, bool>{my_iterator, false};
            }
        }
        // we insert the first node because the tree is empty
        root_node.reset(new Node{std::forward<P>(x)}); // this time we call explicit node constructor (l or r ccording to the type passed with forwarding)
        ++tree_size;
        iterator my_iterator{root_node.get()};
        return std::pair<iterator, bool>{my_iterator, true}; // true because we allocate a new node
    }

    // L-value insert
    std::pair<iterator, bool> insert(const std::pair<const key_type, val_type> &x)
    {
        std::cout << "insert with L" << std::endl;
        return _insert(x);
    }
    // R-value insert
    std::pair<iterator, bool> insert(std::pair<const key_type, val_type> &&x)
    {
        std::cout << "insert with R" << std::endl;
        return _insert(std::move(x));
    }

    // EMPLACE FORWARDING REFERENCE STD::FORWARD _______________________________________________________________________________________
    /*Inserts a new element into the container constructed in-place with
    the given args if there is no element with the key in the container.*/
    template <class... Types>
    std::pair<iterator, bool> emplace(Types &&...args)
    {
        return insert(std::pair<const key_type, val_type>{std::forward<Types>(args)...});
    }

    // CLEAR________________________________________________________________________________________________________________________________________
    void clear()
    {
        root_node.reset(); // we just delete root_node becuase due to unique pointers all the nodes in the tree will be deleted
        tree_size = 0;
    }

    // BALANCE______________________________________________________________________________________________________________________________________________________
    /*Recursive function for balancing the tree.
    Given a sorted vector (in ascending order respect to key) of node's pair (key value),
    this function takes the median (floor rounding) element of the vector
    and inserts the corresponding element in the tree.
    Then, this function recursively calls itself twice:
    one time with the left part of the vector and
    another time with the right part of the vector (median excluded).*/

    void insert_balance(std::vector<std::pair<const key_type, val_type>> &vec, int sindex, int eindex)
    {
        if (sindex == eindex)
        { // there is only one element in the portion of the vector we fall in this when we analyze RIGHT PART
            insert(vec[sindex]);
            return;
        }
        if (sindex > eindex)
        { // we fall in this when we are considering the LEFT PART: WE NEED THIS BCEAUSE THE RESULT OF THE DIVISION IS FLOOR
            return;
        }
        int middle_index = (sindex + eindex) / 2;

        insert(vec[middle_index]);
        insert_balance(vec, sindex, middle_index - 1); // we balance the LEFT PART from 0 to middel-1
        insert_balance(vec, middle_index + 1, eindex); // we balance the RIGHT PART from middle+1 to size-1
    }

    void balance()
    {
        std::vector<std::pair<const key_type, val_type>> vector_pairs;
        for (auto elem : *this)
        { // we are looping trough the object that calls the balance function ( a bst tree and with the range for loop we copy the  std::pairs)
            vector_pairs.push_back(elem);
        }

        clear(); // we clear the bst tree which calls the function balance

        // N.B. pairs of each node in the tree are inserted in the vector in order according to the defintion of the operatro++ for the iterator
        int vector_pairs_size = vector_pairs.size();
        insert_balance(vector_pairs, 0, vector_pairs_size - 1);
    }

    // SUBSCRIPTING OPERATOR____________________________________________________________________________________________________________________________________________________
    /* returns a reference to the value thaht is mapped to a key equivalent to x, performing
    an insertion if such key does not already exist*/

    val_type &operator[](const key_type &x)
    {
        std::cout << "L-value operator[]" << std::endl;
        iterator my_iterator{find(x)};
        if (my_iterator.get_current())// if the current pointer of the iterator is different from nullptr there is a node with that key
        { 
            return my_iterator->second;
        }
        else // create a new pair and insert a new node with that pair in the tree (value is the default)
        {
            insert({x, {}}); 
            return find(x)->second;
        }
    }

    val_type &operator[](key_type &&x)
    {
        std::cout << "R-value operator[]" << std::endl;
        iterator my_iterator{find(x)};
        if (my_iterator.get_current()) // if the current pointer of the iterator is different from nullptr there is a node with that key
        { 
            return my_iterator->second;
        }
        else // create a new pair and insert a new node with that pair in the tree (value is the default)
        {
            insert({x, {}}); 
            return find(x)->second;
        }
    }

    // ERASE_________________________________________________________________________________________________________________________
    /* removes the element (if one exists) with the key equivalent to key passed as argument maintaining the order of the tree*/
    void erase(const key_type &x)
    {

        iterator my_iterator{find(x)};
        Node *remove = my_iterator.get_current(); // we obtain a raw pointer to the current node in which we are with the iterator

        try
        {
            AP_ERROR(remove) << "Node to erase doesn't exist";
        }
        catch (const exception &e)
        {
            std::cout << e.what() << std::endl;
        }

        if (!remove)
        {
            return;
        }

        enum class root_subtree_type
        {
            subtree_root,
            subtree_root_left,
            subtree_root_right
        };

        // RELEASE NODE LMABDA
        /* function that releases the node passed as argument from its parent by releasing the left and right parent's unique pointers.
        It returns a pointer to the released node*/
        auto release_node = [this](const Node *node_to_release)
        {
            if (node_to_release->parent == nullptr) // if the node is a root node we release the root
            {
                return root_node.release();
            }

            if (node_to_release->parent->left.get() == node_to_release) // if the node is a left child we release the parent left pointer
            {
                return node_to_release->parent->left.release();
            }

            return node_to_release->parent->right.release(); // otherwise the node is a right child, we relase the right pointer of the parent
        };

        // GET_LEFT_MOST_FROM_ROOT_SUBTREE LAMBDA
        /* function that returns a pointer to the left most node from a subtree with the root passed as argument*/
        auto get_left_most_from_root_subtree = [](Node *root_subtree) noexcept
        {
            while (root_subtree && root_subtree->left.get())
            {
                root_subtree = root_subtree->left.get();
            }
            return root_subtree;
        };

        // GET_ROOT_SUBTREE_TYPE LAMBDA
        /* function that establishes the kind of node root_subtree that has to be transplanted: root, left child or right_child*/
        auto get_root_subtree_type = [](Node *root_subtree)
        {
            Node *parent = root_subtree->parent;

            if (parent == nullptr)
            {
                return root_subtree_type::subtree_root;
            }
            else if (root_subtree == parent->left.get())
            {
                return root_subtree_type::subtree_root_left;
            }
            else
            {
                return root_subtree_type::subtree_root_right;
            }
        };

        // TRANSPLANT_TREE LAMBDA
        /* function that substitutes a subtree with root root_subtree, with another subtree with root root_graft_subtree
        according to the type of root_subtree determined by the get_root_subtree_type which provides
        the second argument for this function*/
        auto transplant_tree = [this](Node *root_subtree, root_subtree_type rst, Node *root_graft_subtree)
        {
            Node *parent = root_subtree->parent;

            if (rst == root_subtree_type::subtree_root)
            {
                root_node.reset(root_graft_subtree);
            }
            else if (rst == root_subtree_type::subtree_root_left)
            {
                parent->left.reset(root_graft_subtree);
            }
            else
            {
                parent->right.reset(root_graft_subtree);
            }

            if (root_graft_subtree)
            {
                root_graft_subtree->parent = parent;
            }
        };

        if (remove->left.get() == nullptr) // node "remove" have only the right child
        {
            transplant_tree(remove, get_root_subtree_type(remove), remove->right.release());
        }
        else if (remove->right.get() == nullptr) // node "remove" have only the left child
        {
            transplant_tree(remove, get_root_subtree_type(remove), remove->left.release());
        }
        else // node "remove" have both left and right child
        {
            Node *left_most = get_left_most_from_root_subtree(remove->right.get());
            auto left_most_rst = get_root_subtree_type(left_most); /* We need to know the type left_most node, if it is a right or left child.
                                                                   Do that before the release of the node from its parent because after the release we can not access anymore the parent
                                                                   to establish the type of the node*/
            release_node(left_most);

            if (left_most->parent != remove)
            {
                transplant_tree(left_most, left_most_rst, left_most->right.release());
                left_most->right.reset(remove->right.release());
                left_most->right.get()->parent = left_most; // update the parent pointer
            }

            Node *left_child_of_remove = remove->left.release(); /* We save the left_child_of_remove because we need it to update the
                                                                 left pointer of the lef_most node after the transpalnt_tree call. 
                                                                 We can not access anymore the left child of
                                                                 remove by using remove because we release its left pointer.*/

            transplant_tree(remove, get_root_subtree_type(remove), left_most);
            left_most->left.reset(left_child_of_remove);
            left_most->left.get()->parent = left_most; // update the parent pointer
        }

        --tree_size;
    }

    // PUT TO OPERATOR________________________________________________________________________________________________________________
    /* Overloading of the << operator. This function print the
    tree in order using the iterator*/
    friend std::ostream &operator<<(std::ostream &os, const bst &x)
    {
        try
        {
            AP_ERROR(x.root_node) << "Nothing to print the tree is empty"
                                  << ".\n";
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << std::endl;
            os << " " << std::endl;
            return os;
        }

        os << "BST size :[" << x.tree_size << "]\n";
        for (auto v : x)
        {
            os << "N---->key:" << v.first << " value:" << v.second << std::endl;
        }
        os << std::endl;
        return os;
    }
};

// IMPLEMENTATION OF THE ITERATOR
/* Class for the iterator of bst.
According to the bst class we have
-key_type the type of the key to identifying each single node
-val_type the type of the value stored in each node
-OP the operator used to compare the keys of the nodes (default std::less)
- O Type of the object pointed by an instance of _Iterator. WE ARE INTERESTED IN pair_data
*/
template <typename key_type, typename val_type, typename OP>
template <typename O>
class bst<key_type, val_type, OP>::_Iterator
{
    using node = typename bst<key_type, val_type, OP>::Node;
    node *current;

public:
    using value_type = O; // The iterator reference the pair_data, in fact, O can be a const std::pair or a simple std::pair
    using reference = value_type &;
    using pointer = value_type *;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::forward_iterator_tag;

    //_Iteratro constructor
    explicit _Iterator(node *p) : current{p} {}

    // Dereference operator *
    reference operator*() const { return current->pair_data; }

    // Arrow operator ->
    pointer operator->() const { return &**this; }

    // Pre-increment, we move to the next node in the tree considering the order of navigation
    _Iterator &operator++()
    {
        if (!current) // current == nullptr
        {
            return *this;
        }
        else if (current->right) // if the current node has a right child go to the left most node from the right child
        {
            current = current->right.get(); // update current
            while (current->left)
            { // as long as there are left children always move to the left
                current = current->left.get();
            }
        }
        // current = last left node -> return the parent
        // current = last right node -> you reach the end of the tree go back to parents until null pointer
        else // If the current node doesn't have the right node the next node is the first ancestor of the current node, which left son is also an ancestor of the current
        {
            node *tmp = current->parent;

            while (tmp && tmp->right.get() == current)
            {
                current = tmp;
                tmp = tmp->parent;
            }
            current = tmp;
        }
        return *this;
    }

    // Post-Increment
    _Iterator operator++(int)
    {
        auto tmp{*this};
        ++(*this); // call the pre-increment on ourselves
        return tmp;
    }

    // operator ==
    friend bool operator==(const _Iterator &a, const _Iterator &b)
    {
        return a.current == b.current;
    }

    // operator !=
    friend bool operator!=(const _Iterator &a, const _Iterator &b)
    {
        return !(a == b); // we call the previous == operator defined before!!
    }

    // get the current pointer to the node
    node *get_current() { return current; }
};

int main()
{
    int b = 1;
    int *integerpointer = &b;
    integerpointer = nullptr;

    if (!integerpointer)
    {
        std::cout << "il puntatore è NULL" << std::endl;
    }
    else
    {
        std::cout << "il puntatore punta a:" << *integerpointer << std::endl;
    }

    bst<int, int> tree{};

    bst<int, int> tree2{};
    tree2 = tree;
    std::cout << tree << std::endl;
    tree.insert(std::pair<int, int>(6, 1));
    std::cout << tree << std::endl;
    tree.insert(std::pair<int, int>(1, 2));
    std::cout << tree << std::endl;
    tree.insert(std::pair<int, int>(15, 1));
    std::cout << tree << std::endl;
    tree.insert(std::pair<int, int>(12, 2));
    std::cout << tree << std::endl;
    tree.insert(std::pair<int, int>(13, 2));
    std::cout << tree << std::endl;

    // TESTING EMPLACE
    // tree.emplace(5,20);
    // std::cout << tree <<std::endl;

    // TESTING CLEAR
    // tree.clear();
    // std::cout<<tree<<std::endl;

    // TESTING FIND
    // int key = 15;
    // if((*tree.find(key)).first == key ){
    //    std::cout << "there is a node wit the following key:"<< key <<" with value:"<<(*tree.find(key)).second<<std::endl;
    // }

    // TESTING THE OPERATOR []
    // int keyop = 5; //we search for a key which is not present, we simple add a new node with a default value!!
    // auto value_L= tree[keyop]; //L value
    // auto value_R = tree[2]; //R value

    // std::cout<<"value_Lop[]:"<<value_L<<"  value_Rop[]:"<<value_R<<std::endl;

    /*
    //COPY SEMANTIC TEST
    //COPY CONSTRUCTOR
    std::cout<<"TEST copy semantic usign tree"<<std::endl;
    bst<int,int> copy_constructor_tree{tree};
    //COPY ASSIGNMENT
    bst<int, int> copy_assignement_tree{};
    copy_assignement_tree = tree;

    std::cout<<"tree: \n"<<tree<<std::endl;
    std::cout<<"copy constructor tree: \n" << copy_constructor_tree <<std::endl;
    std::cout<<"copy assignment tree: \n"<<copy_assignement_tree<<std::endl;



    //MOVE SEMANTIC TEST
    //MOVE CONSTRUCTOR
    std::cout<<"TEST move semantics"<<std::endl;
    bst<int,int> move_constructor_tree{std::move(tree)};
    std::cout<<"move constructor tree: \n" << move_constructor_tree <<std::endl; //REMEBER TO CHANGE THE NAME IF WE WANT TO TEST MOVE ASSIGNEMNT
    std::cout<<"tree: \n"<<tree<<std::endl;

    //MOVE ASSIGNMENT
    bst<int, int> move_assignement_tree{};
    move_assignement_tree = std::move(copy_constructor_tree); //USED A COPY CONSTRUCTOR
    std::cout<< "move assignment tree: \n"<<move_assignement_tree<<std::endl;
    std::cout<< "copy constructor tree: \n"<<copy_constructor_tree<<std::endl;
    */

    // TESTING ERASE
    tree.erase(6);
    std::cout << tree << std::endl;

    return 0;
}