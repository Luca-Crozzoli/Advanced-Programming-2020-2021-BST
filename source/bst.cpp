#include <algorithm>
#include <iostream>
#include <iterator> // std::forward_iterator_tag;
#include <memory> //std::unique_pytr
#include <utility> // std::move

template<typename key_type, typename value_type, typename OP = std::less<key_type>>
class bst{

    struct Node{
        std::pair<const key_type, value_type> pair_data;
        std::unique_ptr<Node> left;
        std::unique_ptr<Node> right;

        Node* parent; //pointer to the parentNode
        
        //L-VALUE
        //constructor for Node when the tree is empty called by insert
        explicit Node(const std::pair<const key_type,value_type> &x) : pair_data{x}, parent{nullptr} { std::cout << "l-value\n";} 
        //constructor called by insert when the tree has already a node
        Node(const std::pair<const key_type,value_type> &x, Node *p) : pair_data{x}, parent{p} { std::cout << "l-value\n"; }

        //R-VALUE
        //constructor for Node when the tree is empty called by insert 
        explicit Node( std::pair<const key_type,value_type>&&x) : pair_data{std::move(x)}, parent(nullptr) { std::cout << "r-value" << std::endl;}
        //constructor called by insert when the tree has already a node
        Node(std::pair<const key_type,value_type>&&x, Node *p) : pair_data{std::move(x)}, parent{p} {std::cout << "r-value" << std::endl;}

    };

    //COPY FUNCTION CALLED ITERATIVELY TO COPY ALL THE TREE
    void __copy(const std::unique_ptr<Node>& up){
           if (up){//if unique pointer is not empty (it contains the roaw pointer to the root node)
               _insert(up->pair_data);
               __copy(up->left);
               __copy(up->right);
           }
       }

    //INSERT FORWARDING FUNCTION DECLARED AS PRIVATE
    template < typename P>
    std::pair<iterator,bool> _insert(P &&x){ //forwarding is not a n r-value
            auto tmp = root_node.get();

            if(!tmp){//tmp ==nullptr there is no root node
                root_node.reset(new Node{std::forward<P>(x)}); //this time we call the constructor with one argument the pair declared explicit
                return
                /** @todo  implement the retrunr using the iterator over the bst tree*/
                //iterator it{root.get()};
                //return std::pair<iterator, bool>{it,true}; //we return the new node added with an iterator
            }

            //if there is a root node need to loop to decide in which part to insert the new node
            while(tmp){
                if(op(x.first, tmp->pair_data.first))//if x.key < tmp.key (root) retrun true
                {
                    if (!tmp->left) {//tmp->left == nullptr
                        tmp->left.reset(new node{std::forward<P>(x), tmp}); //we call the second constructor in which we set the parent pointer
                        return;
                        /** @todo implment the retrun with the iterator over the bst*/
                    }
                    else
                    {
                        tmp = tmp->left.get();
                    }
                }

                else if(op(tmp->pair_data.first, x.first))//if tmp.key(root) < x.key 
                {
                    if (!tmp->right)// tmp-> right == nullptr
                    {
                        tmp->right.reset(new Node{std::forward<P>(x), tmp});//we call the second constructor in which we set the parent pointer
                        return;
                        /** @todo implement the retrun usign the itrator to the bst tree*/
                    }
                    else
                    {
                        tmp = tmp->right.get();
                    }
                }

                else{ //tmp.key == x.key
                    return;
                    /** @todo retrun nothing , retrun the iteraor at the tmp node there s no need ot inser t
                     * a new node bacuase the key is equal*/

                }
            }
        }
    
    public:
        std::unique_ptr<Node> root_node;
        std::size_t tree_size;
        OP op; //Operator
        
        //DEFAULT CONSTRUCTOR AND DESTRUCTOR
        bst() = default;
        ~bst() noexcept = default; //the default is enough because we don't have raw pointer for the bst class

        //TEMPLATED CLASS FOR THE ITERATOR, I want the iterator to reference the pair data!!
        template <typename O>
        class _Iterator;

        using iterator = _Iterator<std::pair<const key_type, value_type> >; //we pass the templated pair
        using const_iterator = _Iterator<const std::pair<const key_type, value_type> >; // we pass the const templated pair

        //_______________________________________________________________________________________________________________________
        //COPY AND MOVE SEMANTICS FOR THE BST
        /******MOVE default is good because in bst we do not have raw pointers, it is like having a simple copy*/
        bst(bst&&) = default;  //move constructor defualt because we don't have raw pointer
        bst& operator=(bst&&) = default;//move assignment

        /*****COPY default is not good because  in this case we want to perfrom a deep copy*/
        bst(const bst& x){ //copy CONSTRUCTOR
            if (x.tree_size !=0){
                __copy(x.root_node);
            }else{
                std::cout <<"NO ELEMNTS TO COPY"<<std::endl;
            }

        }

        bst& operator=(const bst& x){//copy ASSIGNEMENT a standard way
            root_node.reset(); //remember to release the memory we own

            auto tmp = x; //copy consructor
            *this = std::move(tmp);// move assignment
            return *this;

        }
        //________________________________________________________________________________________________________________________

        //INSERT here we define 2 insert functions calling the private function insert which uses the forwarding:
        //L-value
        std::pair<iterator,bool> insert (const std::pair<const key_type, value_type>& x){return _insert(x);}
        //R-Value
        std::pair<iterator,bool> insert (std::pair<const key_type, value_type>&& x){return _insert(std::move(x));}

        //EMPLACE FORWARDING REFERENCE STD::FORWARD IS NECESSARY

        //CLEAR

        //BEGIN FOR THE ITERATOR noexcept because we are not acquiring resources
        /** @todo retutn the left ost node!!*/
        iterator begin() noexcept {iterator{root_node.get()};}
        const_iterator begin() const noexcept {const_iterator{root_node.get()};}
        const_iterator cbegin() const noexcept {return begin();} 

        //END FOR THE ITERATOR noexcept because we are not acquairing resources
        /** @todo retrun an iteratro to one past the last element*/
        iterattro end() noexcept { iterator{nullptr};}
        const_iterator end() const noexcept {const_iterator{nullptr};}
        const_iteratro cend() const noexcept { retrun end();};

        //FIND FOR THE ITERATOR

        //BALANCE 

        //SUBSCRIPTING OPERATOR

        //PUT TO OPERATOR

        //EREASE

        //OSTREAM
        friend
        std::ostream& operator<<(std::ostream& os, const bst& x){
            auto tmp = x->root_node.get();
            os <<"["<< x.tree_size << "]";
            while(tmp){
                os << tmp->data_pair <<" ";
                /** @todo print the elemnt in order!!!*/
            }    
            os <<std::endl;
            return os;
        }

};

//IMPLEMENTATION OF THE ITERATOR
template< typename key_type, typenme value_type, typename OP>
template <typename O>
class bst<key_type, valuetype, OP>::_Iterator{
    using node = typename bst<key_type, value_type, OP>::Node;
    node* current;

public:
    using value_type = O;//i want the iterator to reference the pair_data, in fact O can be a const std::pair or a simple std::pair
    using reference = value_type&;
    using pointer = value_type*;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::forward_iterator_tag;

    //constructro for the iterator, exlcit because the onstructro can be invoked with one parameter
    explicit _Iterator(const node*p): current{p} {}

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
        return *this
    }

    //post-increment: more computational effort because we need to allocat a new memory
    //and retrun thath value before the increment operation.
    _Iterator operator++(int){
        auto tmp{*this};
        ++(*this); //call the pre increment on ourselfs
        return tmp;
    }

    //operator ==
    friend
    bool operator==(_Iterator& a, _Iterator& b){
        return a.current == b.current;
    }

    //operator !=
    friend
    bool operator!=(_Iterator& a, _Iterator& b){
        return !(a==b); //we call the previous == operatro defined before!!
    }
};

int main(){
    bst<int,int> tree{};
    tree.insert(std::pair<int,int> (8,1));
}