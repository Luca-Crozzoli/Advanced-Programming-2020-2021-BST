#include <algorithm>
#include <iostream>
#include <iterator> // std::forward_iterator_tag;
#include <memory> //std::unique_pytr
#include <utility> // std::move
#include <vector> //std::vector used to create a vector of pair for the balance function
#include "ap_error.h"
using namespace std;

template<typename key_type, typename val_type, typename OP = std::less<key_type>>
class bst{
    struct Node{
        std::pair<const key_type, val_type> pair_data;
        std::unique_ptr<Node> left;
        std::unique_ptr<Node> right;
        Node* parent; 

        Node* get_parent() const noexcept{return parent;}
        Node* get_left() const noexcept{return left.get();}
        Node* get_right() const noexcept{return right.get();}
        
        //L-VALUE 
        /*Constructor for Node called by insert when tree is empty*/
        explicit Node(const std::pair<const key_type,val_type> &x) : pair_data{x}, parent{nullptr} { std::cout << "l-value 1 arg\n";} 
        /*Constructor for Node called by insert when tree is no more empty, set in a proper way the parent pointer*/
        Node(const std::pair<const key_type,val_type> &x, Node *p) : pair_data{x}, parent{p} { std::cout << "l-value 2 arg\n"; }

        //R-VALUE
        /*Constructor for Node called by insert when tree is empty*/
        explicit Node( std::pair<const key_type,val_type>&&x) : pair_data{std::move(x)}, parent(nullptr) { std::cout << "r-value 1 arg" << std::endl;}
        /*Constructor for Node called by insert when tree is no more empty, set in a proper way the parent pointer*/
        Node(std::pair<const key_type,val_type>&&x, Node *p) : pair_data{std::move(x)}, parent{p} {std::cout << "r-value 2 arg" << std::endl;}

        /*Destructor of Node*/
        ~Node() noexcept {std::cout<<"Node destructor"<<std::endl;}

    };

    //COPY FUNCTION CALLED ITERATIVELY TO COPY ALL THE TREE
    /*copy function called iteratively to perfrom a deep copy of the tree when we use the copy constructor
    */
    void __copy(const std::unique_ptr<Node>& up){
        if (up){//if unique pointer != empty (it contains a raw pointer)
            insert(up->pair_data);
            __copy(up->left);
            __copy(up->right);
        }
    }

    public:
        std::unique_ptr<Node> root_node;
        std::size_t tree_size;
        OP op; 
        
        //DEFAULT CONSTRUCTOR AND DESTRUCTOR (default is enoguh because we dont' have raw pointers_______________________________________________________
        bst() = default;
        ~bst() noexcept = default; 

        //TEMPLATED CLASS FOR THE ITERATOR, WE ARE INTERESTED IN O = data_pair const data_pair___________________________________________________________
        template <typename O>
        class _Iterator;

        using iterator = _Iterator<std::pair<const key_type, val_type> >; //we pass the templated pair
        using const_iterator = _Iterator<const std::pair<const key_type, val_type> >; // we pass the const templated pair

        //BEGIN (for the range for loop which uses the iteratro) noexcept because we are not acquiring resources_____________________________________________
        iterator begin() noexcept {
            Node* tmp = root_node.get();
            while(tmp->left){
                tmp = tmp->left.get();
            }
            return iterator{tmp};
        }
        const_iterator begin() const noexcept {
            Node* tmp = root_node.get();
            while(tmp->left){
                tmp = tmp->left.get();
            }
            return const_iterator{tmp};
        }
        const_iterator cbegin() const noexcept {return begin();} 

        //END (for the range for loop which uses the iterator) noexcept because we are not acquairing resources__________________________________________________________________________
        iterator end() noexcept {return iterator{nullptr};}
        const_iterator end() const noexcept { return const_iterator{nullptr};}
        const_iterator cend() const noexcept { return const_iterator{nullptr};};

        //FIND_________________________________________________________________________________________________________________________________________________________
        iterator find(const key_type& x) noexcept{
            for(iterator my_iterator{begin()}; my_iterator != end(); ++my_iterator){
                if(!op(my_iterator->first,x) && !op(x,my_iterator->first)){
                    return my_iterator;
                }
            }
            return end();
        };
        const_iterator find(const key_type& x) const noexcept {
            for(const_iterator my_iterator{begin()}; my_iterator != end(); ++my_iterator){
                if(!op(my_iterator->first,x) && !op(x,my_iterator->first)){
                    return my_iterator;
                }
            }
            return end();
        };

        //COPY AND MOVE SEMANTICS FOR THE BST____________________________________________________________________________________________________________________________
        //Move constructor (default because no raw pointers)
        bst(bst&&) noexcept = default;  
        //move assignment (default because no raw pointers)
        bst& operator=(bst&&) noexcept = default;

        //Copy constructro for a deep copy
        bst(const bst& x): tree_size{} {
        std::cout<<"COPY CONSTRUCTOR"<<std::endl;
            try{
                AP_ERROR(x.root_node)<<"Nothing to copy the tree is empty";
            }catch(const exception& e){
                std::cout<< e.what()<< std::endl;
            }
                __copy(x.root_node);
        }
        //Copy assignment, standard way
        bst& operator=(const bst& x){/
        std::cout<<"COPY ASSIGNMENT"<<std::endl;
            root_node.reset(); //remember to release the memory we own

            auto tmp = x; //copy constructor
            *this = std::move(tmp);// move assignment
            return *this;

        }
        

        //INSERT___________________________________________________________________________________________________________________________________________________________________
        //here we define 2 insert functions calling the function _insert which uses the forwarding to avoid code duplication
        //to take only in one function r and l values
        template <typename P> std::pair<iterator,bool> _insert(P &&x){ //forwarding is not a n r-value
            std::cout<< "forwarding _insert"<<std::endl;
            auto tmp = root_node.get();    
            //if there is a node in the tree we have to loop to decide in which point insert the new one
            while(tmp){
                if(op(x.first, tmp->pair_data.first))//if x.key < tmp.key return true
                {
                    if (!tmp->left) {//tmp->left == nullptr
                        tmp->left.reset(new Node{std::forward<P>(x), tmp}); //we call the second constructor in which we set the parent pointer (l or r according o the type passed with forwarding)
                        ++ tree_size;
                        iterator my_iterator{tmp->left.get()};
                        return std::pair<iterator, bool>{my_iterator, true}; //true because we allocate a new node
                    }
                    else
                    {
                        tmp = tmp->left.get();
                    }
                }
                else if(op(tmp->pair_data.first, x.first))//if x-key > tmp.key
                {
                    if (!tmp->right)// tmp-> right == nullptr
                    {
                        tmp->right.reset(new Node{std::forward<P>(x), tmp});//we call the second constructor in which we set the parent pointer (l or r according o the type passed with forwarding)
                        ++ tree_size;
                        iterator my_iterator{tmp->right.get()};
                        return std::pair<iterator,bool>{my_iterator, true};
                    }
                    else
                    {
                        tmp = tmp->right.get();
                    }
                }

                else{ //tmp.key == x.key
                    iterator my_iterator = find(x.first);
                    return std::pair<iterator, bool>{my_iterator, false}; 
                }
            }
            //we insert the first node because the tree is empty
            root_node.reset(new Node{std::forward<P>(x)}); //this time we call explicit node constructor (l or r ccording to the type passed with forwarding)
                ++ tree_size;
                iterator my_iterator{root_node.get()}; 
                return std::pair<iterator,bool>{my_iterator,true}; //true because we allocate a new node
        }

        //L-value insert
        std::pair<iterator,bool> insert (const std::pair<const key_type, val_type>& x){ std::cout<<"insert with L"<< std::endl; return _insert(x);}
        //R-value insert
        std::pair<iterator,bool> insert (std::pair<const key_type, val_type>&& x){ std::cout<<"insert with R"<< std::endl; return _insert(std::move(x));}

        //EMPLACE FORWARDING REFERENCE STD::FORWARD _______________________________________________________________________________________
        /*Inserts a new element into the container constructed in-place with
        the given args if there is no element with the key in the container.*/ 
        template< class... Types>
        std::pair<iterator,bool> emplace(Types&&...args){
            return insert(std::pair<const key_type, val_type>{std::forward<Types>(args)...});

        }

        //CLEAR________________________________________________________________________________________________________________________________________
        void clear(){
            root_node.reset();//we just delete root_node becuase due to unique pointers all the nodes in the tree will be deleted
            tree_size = 0;
        }

        //BALANCE______________________________________________________________________________________________________________________________________________________
        /*Recursive function for balancing the tree.
        Given a sorted vector (in ascending order respect to key) of node's pair (key value),
        this function takes the median (floor rounding) element of the vector
        and inserts the corresponding element in the tree.
        Then, this function recursively calls itself twice:
        one time with the left part of the vector and
        another time with the right part of the vector (median excluded).*/

        void insert_balance(std::vector<std::pair<const key_type, val_type> >& vec, int sindex, int eindex){
            if(sindex == eindex){//there is only one element in the portion of the vector we fall in this when we analyze RIGHT PART
                insert(vec[sindex]);
                return;
            }
            if(sindex > eindex){//we fall in this when we are considering the LEFT PART: WE NEED THIS BCEAUSE THE RESULT OF THE DIVISION IS FLOOR
                return;
            }
            int middle_index = (sindex + eindex)/2;

            insert(vec[middle_index]);
            insert_balance(vec,sindex,middle_index-1); //we balance the LEFT PART from 0 to middel-1
            insert_balance(vec,middle_index+1,eindex); // we balance the RIGHT PART from middle+1 to size-1

        }
        
        
        void balance(){
            std::vector<std::pair<const key_type,val_type>> vector_pairs;
            for(auto elem: *this){ //we are looping trough the object that calls the balance function ( a bst tree and with the range for loop we copy the  std::pairs)
                vector_pairs.push_back(elem);
            }

            clear(); //we clear the bst tree which calls the function balance

            //N.B. pairs of each node in the tree are inserted in the vector in order according to the defintion of the operatro++ for the iterator
            int vector_pairs_size = vector_pairs.size();
            insert_balance(vector_pairs, 0 , vector_pairs_size-1);
            
        } 

        //SUBSCRIPTING OPERATOR____________________________________________________________________________________________________________________________________________________
        /*retruns a reference to the value thaht is mapped to a key equivalent to x, perfroming
        an insertion if such key does not already exist*/

        val_type& operator[](const key_type& x){
            std::cout<<"L-value operator[]"<<std::endl;
            iterator my_iterator{find(x)};
            if(my_iterator.get_current()){//if current pointer of the iterator is different from nullptr there is a node with that key
                return my_iterator->second;
            }else{
                insert({x,{}});//create a new pair and insert a new node with thah pair in the tree (value is default)
                return find(x)->second;
            }
        }
       
        val_type& operator[](key_type&& x){
            std::cout<<"R-value operator[]"<<std::endl;
            iterator my_iterator{find(x)};
            if(my_iterator.get_current()){//if current pointer of the iteratro is different from nullptr there is a node with that key
                return my_iterator->second;
            }else{
                insert({x,{}});//create a new pair and insert a new node with thath pair in the tree (value is default)
                return find(x)->second;
            }
        }


        //RELEASE NODE
        /*function thath realse the node passed as argument from it's parent by releasing the left and rigth parent's unique pointers.
        It retruns a pointer to the released node*/

        Node* release_node(const Node* node_to_release){
            if(node_to_release->parent == nullptr){ //if the node is a root node we relase the root
                return root_node.release();
            }

            if(node_to_release->parent->left.get() == node_to_release){ //if the node is a left child we release the parent left pointer
                return node_to_release->parent->left.release();
            }

            return node_to_release->parent->right.release();//otherwise the node is a right child, we relase the right pointer of the parent
        }

        //GET_LEFT_MOST
        /* function thaht retrurns a pointer to the left most node from a subtree with the root passed as argument*/
        Node* get_left_most(Node* node) noexcept{
            while(node && node->left.get()){
                node = node->left.get();
            }
            return node;
        }

        //ERASE_________________________________________________________________________________________________________________________
        /*removes the element (if one exists) with the key equivalent ot key mantaining the order of the tree*/
        void erase(const key_type& x){
            
            iterator my_iterator{find(x)};
            Node* remove = my_iterator.get_current(); // we obtain a raw pointer to the current node in which we are with the iterator
           
            try{
                AP_ERROR(remove)<<"Node to erase doesn't exist";
            }catch(const exception& e){
                std::cout<<e.what()<<std::endl;
            }

            if(!remove){
                return;
            }

            enum class root_subtree_type {subtree_root, subtree_root_left, subtree_root_right};
            //LAMBDA FUNCTION THE CAPTURED VARABLES BECOME PARTS OF THE LAMBDA 
            auto get_root_subtree_type = [this](Node* root_subtree){

                Node* parent = root_subtree->parent;

                if(parent == nullptr){
                    return root_subtree_type::subtree_root;
                }else if(root_subtree == parent->left.get()){
                    return root_subtree_type::subtree_root_left;
                }else{
                    return root_subtree_type::subtree_root_right;
                }

            } ;

            //LAMBDA FUNCTION THE CAPTURED VARIABLES BECOME PARTS OF THE LAMBDA
            auto transplant_tree = [this](Node* root_transplant, root_subtree_type rst, Node* transplant){
               Node* parent = root_transplant->parent; 

               if(rst == root_subtree_type::subtree_root){
                   root_node.reset(transplant);
               }else if(rst == root_subtree_type::subtree_root_left){/*else if(remove == parent->left.get())!!____WARNING___!!
                                                                    don't have any more the ownership RETURN NULLPTR!!*/
                   parent->left.reset(transplant);
               }else{
                   parent->right.reset(transplant);
               }

               if(transplant){
                   transplant->parent = parent;
               }
            };

            if(remove->left.get() == nullptr){//Node remove have only right child
                transplant_tree(remove, get_root_subtree_type(remove),remove->right.release());
            }else if(remove->right.get() == nullptr){//node remove have only left child
                transplant_tree(remove,get_root_subtree_type(remove),remove->left.release());
            }
            else{//Node remove have both left and right child
                    Node* left_most = get_left_most(remove->right.get());
                    /*To solve the !!___WARNIING___!! inside the transplant tree function we first need to know
                    the type of the node before it will be relased, to fall in the corret if statement in the transplant
                    function.
                    IF WE DON'T DO THAHT BEFORE THE RELASE WE ENCOUNTER THE WARNING BECAUSE WE ARE ACESSING A NODE
                    WITH GET WHEN WE CALL TRANSPLANT_TREE IN LINE 342 (GET RETRUN NULL BECAUSE THE POINTER WAS PREVIOUSLY RELASED!!)*/
                    auto left_most_rst = get_root_subtree_type(left_most);
                    release_node(left_most); 

                    if(left_most->parent != remove){
                        transplant_tree(left_most,left_most_rst, left_most->right.release());
                        left_most->right.reset(remove->right.release());
                        left_most->right.get()->parent = left_most;
                    }
                    
                    Node* left_child_of_remove = remove->left.release();/* we save the left_child_of_remove  because when we call transplant_tree 
                                                                        the remove node is delted and we can not acess anymore the left child of 
                                                                        remove by using remove.*/
                    
                    transplant_tree(remove,get_root_subtree_type(remove),left_most);
                    left_most->left.reset(left_child_of_remove);
                    left_most->left.get()->parent=left_most;
            }

            --tree_size;
            
            
        }

        //PUT TO OPERATOR________________________________________________________________________________________________________________
        /*Overloading of the << operator. This function print the
        tree in order using the iterator*/
        friend
        std::ostream& operator<<(std::ostream& os, const bst& x){
            try{
                AP_ERROR(x.root_node) << "Nothing to print the tree is empty"<< ".\n";
            }catch(const std::exception& e){
                std::cerr << e.what() << std::endl;
                os << " " << std::endl;
                return os ;
            }
            
            os <<"BST size :["<< x.tree_size << "]\n";
            for(auto v : x){
                os<< "N---->key:"<<v.first<<" value:"<< v.second <<std::endl;
            }
            os <<std::endl;
            return os;
        }

};

//IMPLEMENTATION OF THE ITERATOR
/*Class for the iterator of bst.
According to the bst class we have
-key_type the type of the key to identify each single node 
-val_type the type of the value stored in each node
-OP the operator used to compare the keys of the nodes (default std::less)
- O Type of the object pointed by an instance of _Iterator. WE ARE INTERESTED IN pair_data
*/
template< typename key_type, typename val_type, typename OP>
template <typename O>
class bst<key_type, val_type, OP>::_Iterator{
    using node = typename bst<key_type, val_type, OP>::Node;
    node* current;

public:
    using value_type = O;//i want the iterator to reference the pair_data, in fact O can be a const std::pair or a simple std::pair
    using reference = value_type&;
    using pointer = value_type*;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::forward_iterator_tag;

    //_Iteratro constructor
    explicit _Iterator(node*p): current{p} {}

    //Dereference operator *
    reference operator*() const {return current->pair_data;}

    //Arrow operatro ->
    pointer operator->() const {return &**this;}

    //Pre-increment, we move to th next node in the tree considering the order of navigation
    _Iterator& operator++(){
        if(!current){ //current == nullptr
            return *this;
        }
        else if (current->right){ //if the current node has a right child i have to go to the left most node from the right child
            current = current->right.get(); //return the raw pointer to the right child fo current
            while(current->left){//until there are left child move always on the left
                current = current->left.get();
            }
        }
        //current = last left node -> retrun the parent
        //current = last right node -> you reach the end of the tree go back to parents until null pointer
        else{//If the current node doesn't have right subtree the next node is the first ancestor of current which left son is also an ancestor of current
            node* tmp = current->parent;
            
            while(tmp && tmp->right.get() == current){ //current = last right node in respect to the root -> go back until current = root node
                current = tmp; //move to the father
                tmp = tmp->parent; //tmp will be the grandhfather
            }
            current = tmp;
        }
        return *this;
    }

    //Post-Increment
    _Iterator operator++(int){
        auto tmp{*this};
        ++(*this); //call the pre increment on ourselfs
        return tmp;
    }

    //operator ==
    friend bool operator==(const _Iterator& a, const _Iterator& b){
        return a.current == b.current;
    }

    // operator !=
    friend bool operator!=(const _Iterator& a, const _Iterator& b){
        return !(a == b); //we call the previous == operatro defined before!!
    }

    //get the current pointer to the node
    node* get_current(){return current;}
};

int main(){
    int b = 1;
    int* integerpointer = &b;
    integerpointer = nullptr;

    if(!integerpointer){
        std::cout <<"il puntatore è NULL"<<std::endl;
    }
    else{
        std::cout<< "il puntatore punta a:"<< *integerpointer <<std::endl;
    }
    
    bst<int,int> tree{};

    bst<int,int> tree2{};
    tree2 = tree;
    std::cout<<tree<<std::endl;
    tree.insert(std::pair<int,int> (6,1));
    std::cout << tree <<std::endl;
    tree.insert(std::pair<int,int>(1,2));
    std::cout << tree <<std::endl;
    tree.insert(std::pair<int,int> (15,1));
    std::cout << tree <<std::endl;
    tree.insert(std::pair<int,int>(12,2));
    std::cout << tree <<std::endl;
    tree.insert(std::pair<int,int>(13,2));
    std::cout << tree <<std::endl;

    //TESTING EMPLACE
    //tree.emplace(5,20);
    //std::cout << tree <<std::endl;

    //TESTING CLEAR 
    //tree.clear();
    //std::cout<<tree<<std::endl;
    
    //TESTING FIND 
    //int key = 3;
    //if((*tree.find(key)).first == key ){
    //   std::cout << "there is a node wit the following key:"<< key <<" with value:"<<(*tree.find(key)).second<<std::endl;
    //}

    
    //TESTING THE OPERATOR []
    //int keyop = 5; //we search fro a key which is not present, we simple add a new node with a default value!!
    //auto value_L= tree[keyop]; //L value
    //auto value_R = tree[2]; //R value

    //std::cout<<"value_Lop[]:"<<value_L<<"  value_Rop[]:"<<value_R<<std::endl;


    /*
    //COPY SEMANTIC TEST
    //COPY CONSTRUCTO
    std::cout<<"test copy semantics"<<std::endl;
    bst<int,int> copy_constructor_tree{tree};
    std::cout<<"copy constructor tree: \n" << copy_constructor_tree <<std::endl;
    //COPY ASSIGNMENT
    bst<int, int> copy_assignement_tree{};
    copy_assignement_tree = tree;
    std::cout<< "copy assignment tree: \n"<<copy_assignement_tree<<std::endl;
    
    
    std::cout<<" "<<std::endl;
    std::cout<<"print all the tree create with the copy constructor and the copy assignment witht he originale one \n"<<std::endl;
    std::cout<<"tree: \n"<<tree<<std::endl;
    std::cout<<"copy constructor tree: \n" << copy_constructor_tree <<std::endl;
    std::cout<<"copy assignment tree: \n"<<copy_assignement_tree<<std::endl;
    */

    /*
    //MOVE SEMANTIC TEST
    //MOVE CONSTRUCTOR
    std::cout<<"test move semantics"<<std::endl;
    bst<int,int> move_constructor_tree{std::move(tree)};
    std::cout<<"move constructor tree: \n" << move_constructor_tree <<std::endl;
    
    //MOVE ASSIGNMENT
    //bst<int, int> move_assignement_tree{};
    //move_assignement_tree = std::move(copy_constructor_tree); //USED A COPY CONSTRUCTOR
    //std::cout<< "move assignment tree: \n"<<move_assignement_tree<<std::endl;
    
    
    std::cout<<" "<<std::endl;
    std::cout<<"print all the tree create with the move constructor and the move assignment witht he originale one \n"<<std::endl;
    std::cout<<"move constructor tree: \n" << move_constructor_tree <<std::endl; //REMEBER TO CHANGE THE NAME IF WE WANT TO TST MOVE ASSIGNEMNT
    std::cout<<"tree: \n"<<tree<<std::endl;
    */

    //TESTING ERASE
    //tree.erase(20);

    

   return 0;

}