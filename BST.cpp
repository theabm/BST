#include <iostream>
#include <utility>
#include <memory>
#include <iterator>
#include <algorithm>
#include <vector>

enum class direction {right, left};

template<typename KT, typename VT, typename F = std::less<const KT>>
class BST{

    using PT = std::pair<const KT, VT>;
    template <typename O> class _iterator;
    using iterator = _iterator<PT>;
    using const_iterator = _iterator<const PT>;
    using RT = std::pair<iterator, bool>; 

    
    F f;

    struct node{
        PT value;
        node* parent;
        std::unique_ptr<node> left;
        std::unique_ptr<node> right;
        explicit node(const PT& elem):value{elem}, parent{nullptr}{std::cout<<"l-value node ctor"<< std::endl;}
        explicit node(PT&& elem): value{std::move(elem)},parent{nullptr}{std::cout<<"r-value node ctor"<< std::endl;}
        explicit node(const std::unique_ptr<node>& x, node*p):value{x->value},parent{p}{
            if(x->left){
                left.reset( new node{x->left, this} );
            }
            if(x->right){
                right.reset( new node{x->right, this} );
            }
        }

    };
    std::size_t _size;
    std::unique_ptr<node> head;



    template <typename OT> RT _insert(OT&& x) noexcept { 
        if(!head.get()){  // if BST is empty
            auto _node = new node{std::forward<OT>(x)}; 
            std::cout<<"BST was empty, now contains one node: "<<std::endl;
            head.reset(_node);
            ++_size;
            return RT{iterator{_node}, true}; 
        }
        //BST NOT EMPTY

        auto tmp = head.get();
        auto _node = new node{std::forward<OT>(x)};
        auto val = _node->value.first;
        while(true){
            if (val == tmp->value.first){
                delete _node; 
                return RT{iterator{tmp}, false};
            }
            else if ( f(val, tmp->value.first)){
                if(tmp->left.get()){
                    tmp = tmp -> left.get();
                }
                else{
                    _node->parent = tmp;
                    tmp->left.reset(_node);
                    ++_size;
                    return RT{iterator{_node}, true};
                }
            }
            else{
                if(tmp->right.get()){
                    tmp = tmp -> right.get();
                }
                else{
                    _node->parent = tmp;
                    tmp->right.reset(_node);
                    ++_size;
                    return RT{iterator{_node}, true};
                }

            }
        }

    }

    template <typename OOT> VT& sub(OOT&& x) noexcept {
        auto tmp = insert(PT{std::forward<OOT>(x),{}});
        return tmp.first->second;
    }

    template <typename O> iterator _find(O&& x) noexcept {
        if(!head.get()){
            std::cout<<"BST is empty"<<std::endl;
            return end();
        }
        auto tmp = head.get();
        while(true){
            if ( f(std::forward<O>(x), tmp->value.first)){
                if(tmp->left.get())
                    tmp = tmp -> left.get();
                else{
                    return end();
                }
            }
            else if ( f(tmp->value.first, std::forward<O>(x)) ) {
                if(tmp->right.get())
                    tmp = tmp -> right.get();
                else{
                    return end();
                }
            }
            else{
                return iterator{tmp};
            }
        }
    }


    template <typename O> const_iterator _cfind(O&& x) const noexcept{
        if(!head.get()){
            std::cout<<"BST is empty"<<std::endl;
            return end();
        }

        auto tmp = head.get();
        while(true){
            if ( f(std::forward<O>(x), tmp->value.first) ){
                if(tmp->left.get())
                    tmp = tmp -> left.get();
                else{
                    return end();
                }
            }
            else if ( f(tmp->value.first, std::forward<O>(x)) ) {
                if(tmp->right.get())
                    tmp = tmp -> right.get();
                else{
                    return end();
                }
            }
            else {
                return const_iterator{tmp};
            }
        }
    }
    void delete_leaf(iterator&it, node* dest = nullptr){
        auto parent = it.current->parent;
        it.current->parent = nullptr;
        if(parent->left.get() == it.current)
            parent->left.reset(dest);   
        else
            parent->right.reset(dest);
        if(!dest)
            --_size;
        else
            dest->parent = parent;

        return;

    }

    void one_child_deleter(iterator&it, const direction d ){
        node* tmp;
        switch(d){
            case direction::right:
                tmp = it.current->right.release();
                break;
            case direction::left:
                tmp = it.current->left.release();
                break;
            default:
                break;
        }
        auto parent = it.current->parent;
        it.current->parent = nullptr;
        tmp->parent = parent;
        if(parent->left.get() == it.current)
            parent->left.reset(tmp);
        
        else
            parent->right.reset(tmp);
        
        --_size;
        return;

    }
    

    template <typename O> void _erase(O&&x){
        auto _node = find(std::forward<O>(x));
        std::cout<<"finding: "<<x<<" found: "<<_node<<std::endl;
        if (!_node){ 
            std::cout<<"Erase failed. This key does not exist."<<std::endl;
            return;
        }
        if(!_node.current->left && !_node.current->right){
            std::cout<<"node has no children"<<std::endl;
            delete_leaf(_node);
            return;
        }
        else if(_node.current->left && _node.current->right){

            auto node_pred = _node;
            ++_node; //get successor
            auto newnode = new node{_node.current->value};

            auto left = node_pred.current->left.release();
            newnode->left.reset(left);
            left->parent = newnode;

            auto right = node_pred.current->right.release();
            newnode->right.reset(right);
            right->parent = newnode;

            if(node_pred.current->parent){
                /*auto parent = node_pred.current->parent;
                
                node_pred.current->parent = nullptr;

                if(parent->left.get() == node_pred.current){
                    parent->left.reset(newnode);
                    newnode->parent = parent;
                }
                else{
                    parent->right.reset(newnode);
                    newnode->parent = parent;

                } */
                delete_leaf(node_pred, newnode);

            }
            else{
                head.reset(newnode);
            }
            
            if(!_node.current->left && !_node.current->right ){
                delete_leaf(_node);
                return;
            }
            else{
                if(_node.current->right){
                    direction d{direction::right};
                    one_child_deleter(_node, d);
                }
                else{
                    direction d{direction::left};
                    one_child_deleter(_node, d);
                }
                return;

            }
        }
        else{

            if(_node.current->right){
                direction d{direction::right};
                one_child_deleter(_node, d);

            }
            else{
                direction d{direction::left};
                one_child_deleter(_node, d);

            }
            return;

        }
            
        

            
        
        
    }
        
    void _insert_medians(std::vector<PT> v, int start, int end){
        if(start > end){
            return;
        }
        auto mid = (end+start)/2;

        insert(v[mid]);

        _insert_medians(v, start, mid-1);
        _insert_medians(v, mid+1, end);

    }

    public:

    template <typename ... Types> // packing the types
    RT emplace(Types&& ... args){ //packing the arguments
        return insert(PT{std::forward<Types>(args)...}); //unpack
        
    }

    void clear() noexcept{
        _size = 0;
        head.reset();
    }
    iterator find(const KT& x){return _find(x); }
    iterator find(KT&& x){ return _find(std::move(x)); }
    
    const_iterator find(KT&& x) const {return _cfind(std::move(x)); }
    const_iterator find(const KT& x) const{ return _cfind(x); }



    VT& operator[](const KT& x){ return sub(x); }
    VT& operator[](KT&& x){ return sub(std::move(x)); }
    
    
    auto begin() noexcept{ 
        auto tmp = head.get();
        while (tmp->left)
            tmp = tmp->left.get();
        return iterator{tmp};
    }
    auto begin() const noexcept { 
        auto tmp = head.get();
        while(tmp-> left)
            tmp = tmp->left.get();

        return const_iterator{tmp};
    }

    auto cbegin() const noexcept{
        auto tmp = head.get();
        while(tmp-> left)
            tmp = tmp->left.get();

        return const_iterator{tmp};
    }

    auto end() noexcept { 
        return iterator{nullptr};
    }

    auto end() const noexcept{
        return const_iterator{nullptr};
    }
    
    auto cend() const noexcept{
        return const_iterator{nullptr};
    }


    BST() = default;
    BST(F f): f{std::move(f)}{}; 
    ~BST() = default; 

    // copy semantics 

    BST(const BST &x) : _size{x._size} {
        if(x.head.get()){
            head.reset(new node{x.head, nullptr});
        } 
          
    }
    BST& operator=(BST& x ){
        head.reset(); // reset the head
        auto tmp = x; //copy ctor 
        *this = std::move(tmp); //move assignment
        return *this;
        
    }


    // Move semantics. No raw ptrs so default move is fine.

    BST(BST &&) = default;
    BST& operator=(BST &&) = default;



    RT insert(const PT& x){ return _insert(x); }
    RT insert(PT&& x){ return _insert(std::move(x));}

    void erase(const KT&x){ return _erase(x); }
    void erase(KT&& x){ return _erase(std::move(x)); }
    
    void balance(){
        std::cout<<"ROOT OF TREE: "<<head.get()->value.first<<std::endl;

        std::vector<PT> v;
        v.reserve(_size);
        for(const auto& x: *this){
            v.emplace_back(x);
        }
        /* v.reserve(_size);
        std::uninitiliazed_copy(begin(), end(), v.begin()); */

        clear();

        _insert_medians(v, 0, v.size()-1);
        std::cout<<"ROOT OF NEW BALANCED TREE: "<<head.get()->value.first<<std::endl;

        return;
    }
    


    friend std::ostream &operator<<(std::ostream &os, const BST &x) {
    if(!x.head){
        os << "BST is empty => size: [" <<x._size << "] ";
        os << std::endl;
        return os;
    }
    
    os << "size: [" << x._size << "] ";

    for (const auto& el : x)
        os << el.first << " ";
    os << std::endl;
    return os;
  }

};



template <typename KT, typename VT, typename F> 
template <typename O> 
struct BST<KT,VT,F>::_iterator {
  F f;
  using node = typename BST<KT,VT,F>::node;
  node* current;

// public:
  using value_type = O;
  using reference = value_type &;
  using pointer = value_type *;
  using difference_type = std::ptrdiff_t;
  using iterator_category = std::forward_iterator_tag;

  explicit _iterator(node *p) : current{p} {}

  reference operator*() const { //return current->value.first;
    return current->value;
    //return *current;
   }

  pointer operator->() const { return &**this; }

  // pre-increment
  _iterator &operator++() {
      if(!current){
          return *this;
      }
      if(current->right.get()){ //if right child, go right and then all the way left.
          current = current->right.get();
          while(current->left){
            current = current -> left.get();
          }
          return *this;
        }
      else{ 
          while(current->parent && f(current->parent->value.first,current->value.first)){
              current = current->parent;
          }
          current = current->parent;
          return *this;
        }
    }

    _iterator &operator--() {
      if(!current){
          return *this;
      }
      if(current->left.get()){ //if left child, go left and then all the way right.
          current = current->left.get();
          while(current->right){
            current = current -> right.get();
          }
          return *this;
        }
      else{ //current ->parent is to deal with case when I get to root node and one of the branches is missing
          while(current->parent && f(current->value.first, current->parent->value.first)){
              current = current->parent;
          }
          current = current->parent;
          return *this;
        }
    }



  operator bool() const {return !(current==nullptr); }

  friend bool operator==(_iterator &a, _iterator &b) noexcept {
    return a.current == b.current;
  }

  friend bool operator!=(_iterator &a, _iterator &b) noexcept { 
      return !(a == b); 
      }

  friend std::ostream &operator<<(std::ostream &os, const _iterator &x) {
    os << "node: [" << x.current << "] ";
    os << "value: " <<x.current->value.first << " parent: "<< x.current -> parent;
    os << " left: "<< x.current ->left.get() << " right: "<< x.current -> right.get();

    
    
    os << std::endl;
    return os;
  }


};


int main(){

    BST<int,int> bst{};
    using PT = std::pair<int,int>; 
    PT a{8,1};
    PT b{3,4};
    PT c{10,55};
    PT d{1,55};
    PT e{6,55};
    PT j{4,11};
    PT f{7,33};
    PT g{14,33};
    PT h{13,23};

    auto tmp = bst.insert(a);
    tmp = bst.insert({3,4}); //inserting r-value
    tmp = bst.emplace(10,55); //emplace
    tmp = bst.insert(d);
    tmp = bst.insert(e);
    tmp = bst.emplace(j);
    tmp = bst.insert(f);
    tmp = bst.insert(g);
    tmp = bst.insert(h);

    for (auto& x: bst)
        std::cout<<x.first<<std::endl;

    std::cout<<bst<<std::endl;


    bst.erase(6);
    std::cout<<bst<<std::endl;

    return 0;

    // INSERT DONE
    // EMPLACE DONE
    

    //bst.clear();
    //std::cout<<bst<<std::endl;

    // CLEAR DONE
    { 
    auto ff = bst.find(8); 
    if(ff){ 
        std::cout<<ff<<"  "<<ff->first<<std::endl;
    }
    else
        std::cout<<ff<<std::endl;
    }

    { 
    int tmp{7};    
    auto ff = bst.find(tmp);
    if(ff){ 
        std::cout<<ff<<"  "<<ff->first<<std::endl;
    }
    else
        std::cout<<ff<<std::endl;
    } 

    { 
    auto ff = bst.find(5);
    if(ff){ 
        std::cout<<ff<<"  "<<ff->first<<std::endl;
    }
    else
        std::cout<<ff<<std::endl;
    } 


    // FIND DONE

    bst.balance();

    std::cout<<bst<<std::endl;

    // BALANCE DONE
    {
    auto tmp = bst[3];
    std::cout<<tmp<<std::endl;
    }
    {
    auto tmp = bst[9];
    std::cout<<tmp<<std::endl;
    }
    {
    auto tmp1 = 10;
    auto tmp = bst[tmp1];
    std::cout<<tmp<<std::endl;

    }

    std::cout<<bst<<std::endl;

    // SUBSCRIPT DONE

    // PUT TO OPERATOR DONE
    std::cout<<"copy ctor:"<<std::endl;
    {
    BST<int,int> bst2{bst};

    std::cout<<"bst is: "<<bst<<"\n"<<"bst2 is: "<<bst2<<std::endl;

    bst.emplace(-2,2);
    bst.emplace(11,3);
    bst2.emplace(-5,22);
    bst2.emplace(-7, 2);
    bst2.emplace(-1,0);
    std::cout<<"after insertions"<<std::endl;
    std::cout<<"bst is: "<<bst<<"\n"<<"bst2 is: "<<bst2<<std::endl;
    }
    std::cout<<"copy assignment:"<<std::endl;

    {
    BST<int,int> bst2;
    bst2 = bst;

    std::cout<<"bst is: "<<bst<<"\n"<<"bst2 is: "<<bst2<<std::endl;

    bst.emplace(-2,2);
    bst.emplace(11,3);
    bst2.emplace(-5,22);
    bst2.emplace(-7, 2);
    bst2.emplace(-1,0);
    std::cout<<"after insertions"<<std::endl;
    std::cout<<"bst is: "<<bst<<"\n"<<"bst2 is: "<<bst2<<std::endl;
    bst2.balance();

    }
    // COPY DONE

    //{
        //BST<int,int> bst2;
        //bst2 = std::move(bst);
        //std::cout<<"bst2 is: "<<bst2<<std::endl;
    //}
    
    // TESTED MOVE WITH BOTH MOVE CTOR AND MOVE ASSINGMENT. MOVE DONE


    bst.erase(7);
    return 0;


}
