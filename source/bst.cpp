#include <algorithm>
#include <iostream>
#include <iterator> // std::forward_iterator_tag;
#include <memory> //std::unique_pytr
#include <utility> // std::move
using namespace std;

template<typename key_type, typename val_type, typename OP = std::less<key_type>>
class bst{

    struct Node{
        std::pair<const key_type, val_type> pair_data;
        std::unique_ptr<Node> left;
        std::unique_ptr<Node> right;

        Node* parent; //pointer to the parentNode
        
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

        //TEMPLATED CLASS FOR THE ITERATOR, WE ARE ITERATE OVER THE data_pair!!_____________________________
        template <typename O>
        class _Iterator;

        using iterator = _Iterator<std::pair<const key_type, val_type> >; //we pass the templated pair
        using const_iterator = _Iterator<const std::pair<const key_type, val_type> >; // we pass the const templated pair

        //BEGIN FOR THE ITERATOR noexcept because we are not acquiring resources_____________________________________________
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
            return cend();
        };

        //_______________________________________________________________________________________________________________________
        //COPY AND MOVE SEMANTICS FOR THE BST
        /******MOVE default is good because in bst we do not have raw pointers, it is like having a simple copy*/
        bst(bst&&) = default;  //move CONSTRUCTOR defualt because we don't have raw pointer
        bst& operator=(bst&&) = default;//move ASSIGNMENT

        /*****COPY default is not good because  in this case we want to perfrom a deep copy*/
        bst(const bst& x): tree_size{} { //copy CONSTRUCTOR
        std::cout<<"COPY CONSTRUCTOR"<<std::endl;
            if (x.tree_size !=0){
                __copy(x.root_node);
            }else{
                std::cout <<"NO ELEMENTS TO COPY"<<std::endl;
            }

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
            //if there is a node we have to loop to decide in which point insert the new one
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
                    iterator my_iterator{tmp};
                    return std::pair<iterator, bool>{my_iterator, false}; 
                    /** @todo in this case we also have to find the node with the same key, we will calle the function
                     * FIND inside the else statement
                    */

                }
            }
            root_node.reset(new Node{std::forward<P>(x)}); //this time we call  explicit node constructor (l or r ccording to the type passed with forwarding)
                ++ tree_size;
                iterator my_iterator{root_node.get()}; 
                return std::pair<iterator,bool>{my_iterator,true}; //true because we allocate a new node
        }

        //L-value
        std::pair<iterator,bool> insert (const std::pair<const key_type, val_type>& x){ std::cout<<"insert with L"<< std::endl; return _insert(x);}
        //R-Value
        std::pair<iterator,bool> insert (std::pair<const key_type, val_type>&& x){ std::cout<<"insert with R"<< std::endl; return _insert(std::move(x));}

        //EMPLACE FORWARDING REFERENCE STD::FORWARD IS NECESSARY_______________________________________________________________________________________
        template< class... Types>
        std::pair<iterator,bool> emplace(Types&&...args){
            return insert(std::pair<const key_type, val_type>{std::forward<Types>(args)...});

        }

        //CLEAR________________________________________________________________________________________________________________________________________

        //BALANCE 

        //SUBSCRIPTING OPERATOR

        //PUT TO OPERATOR

        //EREASE

        //PUT TO OPERATOR
        friend
        std::ostream& operator<<(std::ostream& os, const bst& x){
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

    //pre-increment accordin to the navigation fo the tree from left to right
    _Iterator& operator++(){
        if(!current){  //current == nullptr
            return *this;
        }
        //Go right and find the left most node if thera are left node
        else if (current->right){ //if the current node has a right child i have to go to teh left most node from the roght child
            current = current->right.get(); //retrun the raw pointer to the right child fo current
            while(current->left){// until there are left child move always on the left
                current = current->left.get();
            }
        }
        //current = last left node -> retrun the parent
        //current = last right node -> you reach the end of the tree go back to parents until null pointer
        else{
            node* tmp = current->parent;
            //curetn = last rigth node -> go back until current = root node and return nullptr
            while(tmp && tmp->right.get() == current){
                current = tmp; //move to the father
                tmp = current->parent; //tmp will be the grandhfather
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
    tree.insert(std::pair<int,int> (8,1));
    std::cout << tree <<std::endl;
    tree.insert(std::pair<int,int>(9,2));
    std::cout << tree <<std::endl;
    tree.insert(std::pair<int,int> (1,1));
    std::cout << tree <<std::endl;
    tree.insert(std::pair<int,int>(3,2));
    std::cout << tree <<std::endl;

    std::pair<int, int> my_pair(4,1000); //constucto a pair and then insert it
    tree.insert(my_pair);
    std::cout << tree <<std::endl;
    tree.insert(std::make_pair(12,12000)); //insert a pair directly constructing it
    std::cout << tree <<std::endl;
    tree.insert({2,4});
    std::cout << tree <<std::endl;
    tree.emplace(100,123456789);
    std::cout << tree <<std::endl;

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
    int key = 3;
    if((*tree.find(key)).first == key ){
        std::cout << "there is a node wit the following key:"<< key <<" with value:"<<(*tree.find(key)).second<<std::endl;
    }

   return 0;

}