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

        Node* parent; //pointer to the parentNode

        Node* get_parent() const noexcept{return parent;}
        Node* get_left() const noexcept{return left.get();}
        Node* get_right() const noexcept{return right.get();}
        
        //L-VALUE CNSTRUCTORS FOR THE COPY SEMANTIC
        //constructor for Node when the tree is empty called by insert
        explicit Node(const std::pair<const key_type,val_type> &x) : pair_data{x}, parent{nullptr} { std::cout << "l-value 1 arg\n";} 
        //constructor called by insert when the tree has already a node
        Node(const std::pair<const key_type,val_type> &x, Node *p) : pair_data{x}, parent{p} { std::cout << "l-value 2 arg\n"; }

        //R-VALUE
        //constructor for Node when the tree is empty called by insert 
        explicit Node( std::pair<const key_type,val_type>&&x) : pair_data{std::move(x)}, parent(nullptr) { std::cout << "r-value 1 arg" << std::endl;}
        //constructor called by insert when the tree has already a node
        Node(std::pair<const key_type,val_type>&&x, Node *p) : pair_data{std::move(x)}, parent{p} {std::cout << "r-value 2 arg" << std::endl;}

        ~Node() noexcept {std::cout<<"Node destructor"<<std::endl;}

    };

    //COPY FUNCTION CALLED ITERATIVELY TO COPY ALL THE TREE
    void __copy(const std::unique_ptr<Node>& up){
        if (up){//if unique pointer is not empty (it contains a raw pointer)
            insert(up->pair_data);
            __copy(up->left);
            __copy(up->right);
        }
    }

    public:
        std::unique_ptr<Node> root_node;
        std::size_t tree_size;
        OP op; //Operator
        
        //DEFAULT CONSTRUCTOR AND DESTRUCTOR____________________________________________________________________________
        bst() = default;
        ~bst() noexcept = default; //the default is enough because we don't have raw pointer for the bst class

        //TEMPLATED CLASS FOR THE ITERATOR, WE ARE INTERESTED IN THE data_pair!!_____________________________
        template <typename O>
        class _Iterator;

        using iterator = _Iterator<std::pair<const key_type, val_type> >; //we pass the templated pair
        using const_iterator = _Iterator<const std::pair<const key_type, val_type> >; // we pass the const templated pair

        //BEGIN FOR THE ITERATOR noexcept because we are not acquiring resources_____________________________________________
        //we start from the left most node in the tree which is the first thath has to be readed.
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

        //END FOR THE ITERATOR noexcept because we are not acquairing resources_________________________________________________
        iterator end() noexcept {return iterator{nullptr};}
        const_iterator end() const noexcept { return const_iterator{nullptr};}
        const_iterator cend() const noexcept { return const_iterator{nullptr};};

        //FIND FOR THE ITERATOR_________________________________________________________________________________________________
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

        //_______________________________________________________________________________________________________________________
        //COPY AND MOVE SEMANTICS FOR THE BST
        /******MOVE default is good because in bst we do not have raw pointers, it is like having a simple copy*/
        bst(bst&&) noexcept = default;  //move CONSTRUCTOR defualt because we don't have raw pointer
        bst& operator=(bst&&) noexcept = default;//move ASSIGNMENT

        /*****COPY default is not good because  in this case we want to perfrom a deep copy*/
        bst(const bst& x): tree_size{} { //copy CONSTRUCTOR
        std::cout<<"COPY CONSTRUCTOR"<<std::endl;
            try{
                AP_ERROR(x.root_node)<<"Nothing to copy the tree is empty";
            }catch(const exception& e){
                std::cout<< e.what()<< std::endl;
            }
                __copy(x.root_node);
        }

        bst& operator=(const bst& x){//copy ASSIGNEMENT a standard way
        std::cout<<"COPY ASSIGNMENT"<<std::endl;
            root_node.reset(); //remember to release the memory we own

            auto tmp = x; //copy constructor
            *this = std::move(tmp);// move assignment
            return *this;

        }
        //________________________________________________________________________________________________________________________

        //INSERT here we define 2 insert functions calling the function_insert which uses the forwarding:
        //INSERT FORWARDING FUNCTION DECLARED AS PRIVATE AND DEFINED OUTSIDE THE CLASS AFTER THE ITERATOR
        template <typename P> std::pair<iterator,bool> _insert(P &&x){ //forwarding is not a n r-value
            std::cout<< "forwarding _insert"<<std::endl;
            auto tmp = root_node.get();    
            //if there is a node in the tree we have to loop to decide in which point insert the new one
            while(tmp){
                if(op(x.first, tmp->pair_data.first))//*********if x.key < tmp.key return true
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
                else if(op(tmp->pair_data.first, x.first))//*********if x-key > tmp.key
                {
                    if (!tmp->right)// tmp-> right == nullptr
                    {
                        tmp->right.reset(new Node{std::forward<P>(x), tmp});//we call the second constructor in which we set the parent pointer
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
            root_node.reset(new Node{std::forward<P>(x)}); //this time we call  explicit node constructor (l or r ccording to the type passed with forwarding)
                ++ tree_size;
                iterator my_iterator{root_node.get()}; 
                return std::pair<iterator,bool>{my_iterator,true}; //true because we allocate a new node
        }

        //L-value
        std::pair<iterator,bool> insert (const std::pair<const key_type, val_type>& x){ std::cout<<"insert with L"<< std::endl; return _insert(x);}
        //R-Value
        std::pair<iterator,bool> insert (std::pair<const key_type, val_type>&& x){ std::cout<<"insert with R"<< std::endl; return _insert(std::move(x));}

        //EMPLACE FORWARDING REFERENCE STD::FORWARD IS NECESSARY HERE IS VARIADIC FUNCTION TEMPLATE_______________________________________________________________________________________
        template< class... Types>
        std::pair<iterator,bool> emplace(Types&&...args){
            return insert(std::pair<const key_type, val_type>{std::forward<Types>(args)...});

        }

        //CLEAR________________________________________________________________________________________________________________________________________
        void clear(){
            root_node.reset();//resetting the root node allo th edeletion of all the other nodes because fo smart pointers!
            tree_size = 0;
        }

        //BALANCE______________________________________________________________________________________________________________________________________________________
        void insert_balance(std::vector<std::pair<const key_type, val_type> >& vec, int sindex, int eindex){
            if(sindex == eindex){//there is only one lement in the portion of the vector we fall in this when we analize RIGHT PART
                insert(vec[sindex]);
                return;
            }
            if(sindex > eindex){//we fall in this when we are considering the LEFT PART: WE NEED TIS BCEAUSE THE RESULT OF THE DIVISION IS FLOOR
                return;
            }
            int middle_index = (sindex + eindex)/2;

            insert(vec[middle_index]);
            insert_balance(vec,sindex,middle_index-1); //we balance the LEFT PART from 0 to middel-1
            insert_balance(vec,middle_index+1,eindex); // we balance the RIGHT PART from middle+1 to size-1

        }
        
        void balance(){
            std::vector<std::pair<const key_type,val_type>> vector_pairs;
            for(auto elem: *this){ //we are looping trough the object thhat calls the blance function ( a bst tree and with the range for loop we copy the  std::pairs)
                vector_pairs.push_back(elem);
            }

            clear(); //we clear the bst tree which calls the function balance.

            //SORTING THE VECTRO IT'S AN ADDITIONAL OPERATION BECAUSE IN THE RANGE FOR LOOP WE USED THE ITERATOR
            //WHICH (AS WE IMPLEMENTED IT) RETURNS THE VALUE IN ORDER WHILE WE NAVIGATE TROUGH THE TREE
            /** @todo remeber to delete the sort function because the iteratr is alredy navigate trogh the tree following the key order*/
            sort(vector_pairs.begin(), vector_pairs.end()); // we sort the vector in ascending order. By defaule sort() in a pair vectro use the first element to order!
            int vector_pairs_size = vector_pairs.size();
            insert_balance(vector_pairs, 0 , vector_pairs_size-1);
            
        } 

        //SUBSCRIPTING OPERATOR____________________________________________________________________________________________________________________________________________________
        val_type& operator[](const key_type& x){
            std::cout<<"L-value operator[]"<<std::endl;
            iterator my_iterator{find(x)};
            if(my_iterator.get_current()){//if current pointer of the iteratro is different from nullptr there is a node with that key
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


        //ADDITIONAL FUNCTION USED TO REALSE THE NODE, we reales the node from itìs parent
        Node* release_node(const Node* node_to_release){
            if(node_to_release->parent == nullptr){ //if the node is a root node we relase the root
                return root_node.release();
            }

            if(node_to_release->parent->left.get() == node_to_release){ //if the node is a left child we releeas the parent left pointer
                return node_to_release->parent->left.release();
            }

            return node_to_release->parent->right.release();//otherwise the node is a right child, we relase the right pointer of the parent
        }


        Node* get_left_most(Node* node) noexcept{
            while(node && node->left.get()){
                node = node->left.get();
            }
            return node;
        }

        //EREASE_________________________________________________________________________________________________________________________
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
            //LAMBDA FUNCTION, WE EXPLOIT THE CLOSURE CONCEPT, THE CAPTURED VARABLES BECOME PARTS OF THE LAMBDA 
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

            //LAMBDA FUNCTION, WE EXPLOIT THE CLOSURE CONCEPT, THE CAPTURED VARIABLE BECOME PARTS OF THE LAMBDA
            auto transplant_tree = [this](Node* root_transplant, root_subtree_type rst, Node* transplant){ //lambda function!! [this,remove] variable included in the scpe of the function
               Node* parent = root_transplant->parent; 

               if(rst == root_subtree_type::subtree_root){
                   root_node.reset(transplant);
               }else if(rst == root_subtree_type::subtree_root_left){/**else if(remove == parent->left.get())!!____WARNING___!!non ho più la ownership MI RITORNA UN NULLPTR!!*/
                   parent->left.reset(transplant);
               }else{
                   parent->right.reset(transplant);
               }

               if(transplant){
                   transplant->parent = parent;
               }
            };

            if(remove->left.get() == nullptr){//Node remove have only rigth child
                transplant_tree(remove, get_root_subtree_type(remove),remove->right.release());
            }else if(remove->right.get() == nullptr){//node remove have only left child
                transplant_tree(remove,get_root_subtree_type(remove),remove->left.release());
            }
            else{//Node remove have both left and right child
                    Node* left_most = get_left_most(remove->right.get());
                    //To slve the !!___WARNIING___!! inside the transplat tree function we firt nedd to know
                    //the type of the node before it will be relased to fall in the corret if statement in the transplant
                    //function.
                    //IF WE DON'T DO THAHT BEFORE THE RELASE WE ENCOUNTER THE WARNING BECAUSE WE ARE ACESSING A NODE
                    //WITH GET ALREADY REALS AND IT WILL RETURN A NULLPTR
                    auto left_most_rst = get_root_subtree_type(left_most);
                    release_node(left_most); //we release the node to avoid the dleetion of the full tree

                    if(left_most->parent != remove){
                        transplant_tree(left_most,left_most_rst, left_most->right.release());
                        left_most->right.reset(remove->right.release());
                        left_most->right.get()->parent = left_most;
                    }
                    
                    Node* left_child_of_remove = remove->left.release();
                    
                    transplant_tree(remove,get_root_subtree_type(remove),left_most);
                    left_most->left.reset(left_child_of_remove);
                    left_most->left.get()->parent=left_most;
            }

            --tree_size;
            
            
        }

        //PUT TO OPERATOR________________________________________________________________________________________________________________
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

    //constructro for the iterator, exlcit because the constructro can be invoked with one parameter
    explicit _Iterator(node*p): current{p} {}

    //* and -> operators
    reference operator*() const {return current->pair_data;}
    pointer operator->() const {return &**this;}

    //pre-increment according to the navigation of the tree from left to right
    _Iterator& operator++(){
        if(!current){  //current == nullptr
            return *this;
        }
        //If te current node has a right substree then the next node is the left most to the right subtree
        else if (current->right){ //if the current node has a right child i have to go to teh left most node from the roght child
            current = current->right.get(); //retrun the raw pointer to the right child fo current
            while(current->left){// until there are left child move always on the left
                current = current->left.get();
            }
        }
        //current = last left node -> retrun the parent
        //current = last right node -> you reach the end of the tree go back to parents until null pointer
        else{//If the current node doesn't have right subtree the next node is the first ancestor of current which left sono is also an ancestor of current
            node* tmp = current->parent;
            //current = last rigth node in respect to the root -> go back until current = root node 
            while(tmp && tmp->right.get() == current){
                current = tmp; //move to the father
                tmp = tmp->parent; //tmp will be the grandhfather
            }
            current = tmp;
        }
        return *this;
    }

    //post-increment: more computational effort because we need to allocat a new memory
    //and retrun thath value before the increment operation.
    _Iterator operator++(int){
        auto tmp{*this};
        ++(*this); //call the pre increment on ourselfs
        return tmp;
    }

    //operator ==
    friend bool operator==(const _Iterator& a, const _Iterator& b){
        return a.current == b.current;
    }

    //operator !=
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


    //TESTING ERASE
    tree.erase(20);

    //TESTING EMPLACE
    //tree.emplace(1,20);

    std::cout << tree <<std::endl;

    

    //TESTING THE OPERATRO []
    //int keyop = 5; //we search fro a key which is not present, we simple add a new node with a default value!!
    //auto value_L= tree[keyop]; //L value
    //auto value_R = tree[2]; //R value

    //std::cout<<"value_Lop[]:"<<value_L<<"  value_Rop[]:"<<value_R<<std::endl;



    //TESTING CLEAR COMMAND
    //tree.clear();
    //std::cout<<tree<<std::endl;

    //DIVISION WITH INTEGERS IT RETRUNS THE FLOOR VALUE (AROTONDAMENTO PER DIFETTO)
    //std::cout<<"Performing an integer division"<<std::endl;
    //int a = 1;
    //int d = 2;
    //std::cout<<"the result of "<<a<<"/"<<d<<" = "<<a/d<<std::endl;

    //int value = 0-1;
    //std::cout<<"0-1="<<value<<std::endl;

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
    std::cout<<"copy assignment tree: \n"<<copy_assignement_tree<<std::endl;*/

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
    std::cout<<"tree: \n"<<tree<<std::endl;*/

    //TESTING FIND FUNCTION
    /** @todo we need to manage the exception when there is no node thath can be found having thath key!
     * @exception Segmentation fault segmentation fault if we can not find the key in the tree!!
    */
    //int key = 3;
    //if((*tree.find(key)).first == key ){
    //    std::cout << "there is a node wit the following key:"<< key <<" with value:"<<(*tree.find(key)).second<<std::endl;
    //}

   return 0;

}