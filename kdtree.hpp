#include <tuple>
#include <vector>
#include <algorithm>
#include <cassert>
#include <stdexcept>

/**
 * An abstract template base of the KDTree class
 */
template<typename...>
class KDTree;

/**
 * A partial template specialization of the KDTree class
 * The time complexity of functions are based on n and k
 * n is the size of the KDTree
 * k is the number of dimensions
 * @typedef Key         key type
 * @typedef Value       value type
 * @typedef Data        key-value pair
 * @static  KeySize     k (number of dimensions)
 */
template<typename ValueType, typename... KeyTypes>
class KDTree<std::tuple<KeyTypes...>, ValueType> {
public:
    typedef std::tuple<KeyTypes...> Key;
    typedef ValueType Value;
    typedef std::pair<const Key, Value> Data;
    static inline constexpr size_t KeySize = std::tuple_size<Key>::value;
    static_assert(KeySize > 0, "Can not construct KDTree with zero dimension");
protected:
    struct Node {
        Data data;
        Node *parent;
        Node *left = nullptr;
        Node *right = nullptr;

        Node(const Key &key, const Value &value, Node *parent) : data(key, value), parent(parent) {}

        const Key &key() { return data.first; }

        Value &value() { return data.second; }
    };

public:
    /**
     * A bi-directional iterator for the KDTree
     * ! ONLY NEED TO MODIFY increment and decrement !
     */
    class Iterator {
    private:
        KDTree *tree;
        Node *node;

        Iterator(KDTree *tree, Node *node) : tree(tree), node(node) {}

        /**
         * Increment the iterator
         * Time complexity: O(log n)
         */
        void increment() {
            // TODO: implement this function
            Node* successor;
            if (node->right!=nullptr) {
                successor = node->right;
                while (successor->left!=nullptr) {
                    successor = successor->left;
                }
                node = successor;
            } else {
                successor = node;
                Node* successor_parent;
                while (successor->parent!=nullptr) {
                    successor_parent = successor->parent;
                    if (successor==successor_parent->left) {
                        successor = successor_parent;
                        node = successor;
                        return;
                    }
                    successor = successor_parent;
                }
                node = nullptr;
            }
            return;
        }

        /**
         * Decrement the iterator
         * Time complexity: O(log n)
         */
        void decrement() {
            // TODO: implement this function
            Node* predecessor;
            if (node->left) {
                predecessor = node->left;
                while (predecessor->right) {
                    predecessor = predecessor->right;
                }
                node = predecessor;
            } else {
                predecessor = node;
                Node* predecessor_parent;
                while (predecessor->parent) {
                    predecessor_parent = predecessor->parent;
                    if (predecessor==predecessor_parent->right) {
                        predecessor = predecessor_parent;
                        node = predecessor;
                        return;
                    }
                    predecessor = predecessor_parent;
                }
                node = nullptr;
            }
            return;
        }

    public:
        friend class KDTree;

        Iterator() = delete;

        Iterator(const Iterator &) = default;

        Iterator &operator=(const Iterator &) = default;

        Iterator &operator++() {
            increment();
            return *this;
        }

        Iterator operator++(int) {
            Iterator temp = *this;
            increment();
            return temp;
        }

        Iterator &operator--() {
            decrement();
            return *this;
        }

        Iterator operator--(int) {
            Iterator temp = *this;
            decrement();
            return temp;
        }

        bool operator==(const Iterator &that) const {
            return node == that.node;
        }

        bool operator!=(const Iterator &that) const {
            return node != that.node;
        }

        Data *operator->() {
            return &(node->data);
        }

        Data &operator*() {
            return node->data;
        }
    };

protected:                      // DO NOT USE private HERE!
    Node *root = nullptr;       // root of the tree
    size_t treeSize = 0;        // size of the tree

    /**
     * Find the node with key
     * Time Complexity: O(k log n)
     * @tparam DIM current dimension of node
     * @param key
     * @param node
     * @return the node with key, or nullptr if not found
     */
    template<size_t DIM>
    Node *find(const Key &key, Node *node) {
        constexpr size_t DIM_NEXT = (DIM + 1) % KeySize;
        // TODO: implement this function
        if (!node) return nullptr;
        if (key==node->key()) return node;
        else if (std::get<DIM>(node->key()) > std::get<DIM>(key)) {
            return find<DIM_NEXT>(key, node->left);
        } else {
            return find<DIM_NEXT>(key, node->right);
        } 
    }

    /**
     * Insert the key-value pair, if the key already exists, replace the value only
     * Time Complexity: O(k log n)
     * @tparam DIM current dimension of node
     * @param key
     * @param value
     * @param node
     * @param parent
     * @return whether insertion took place (return false if the key already exists)
     */
    template<size_t DIM>
    bool insert(const Key &key, const Value &value, Node *&node, Node *parent) {
        constexpr size_t DIM_NEXT = (DIM + 1) % KeySize;
        // TODO: implement this function
        if (!node) {
            node = new Node(key, value, parent);
            treeSize++;
            return true;
        }
        if (key==node->key()) {
            node->value() = value;
            return false;
        } else if (std::get<DIM>(node->key()) > std::get<DIM>(key)) {
            return insert<DIM_NEXT>(key, value, node->left, node);
        }  else {
            return insert<DIM_NEXT>(key, value, node->right, node);
        }
    }

    /**
     * Compare two keys on a dimension
     * Time Complexity: O(1)
     * @tparam DIM comparison dimension
     * @tparam Compare
     * @param a
     * @param b
     * @param compare
     * @return relationship of two keys on a dimension with the compare function
     */
    template<size_t DIM, typename Compare>
    static bool compareKey(const Key &a, const Key &b, Compare compare = Compare()) {
        if (std::get<DIM>(a) != std::get<DIM>(b)){
            return compare(std::get<DIM>(a), std::get<DIM>(b));
        }
        return compare(a, b);
    }

    /**
     * Compare two nodes on a dimension
     * Time Complexity: O(1)
     * @tparam DIM comparison dimension
     * @tparam Compare
     * @param a
     * @param b
     * @param compare
     * @return the minimum / maximum of two nodes
     */
    template<size_t DIM, typename Compare>
    static Node *compareNode(Node *a, Node *b, Compare compare = Compare()) {
        if (!a) return b;
        if (!b) return a;
        return compareKey<DIM, Compare>(a->key(), b->key(), compare) ? a : b;
    }

    /**
     * Find the minimum node on a dimension
     * Time Complexity: ?
     * @tparam DIM_CMP comparison dimension
     * @tparam DIM current dimension of node
     * @param node
     * @return the minimum node on a dimension
     */
    template<size_t DIM_CMP, size_t DIM>
    Node *findMin(Node *node) {
        constexpr size_t DIM_NEXT = (DIM + 1) % KeySize;
        // TODO: implement this function
        if (!node) return nullptr;
        Node* min = findMin<DIM_CMP, DIM_NEXT>(node->left);
        if (DIM_CMP!=DIM) {
            Node* rightMin = findMin<DIM_CMP, DIM_NEXT>(node->right);
            min = compareNode<DIM_CMP, std::less<>>(min, rightMin);
        }
        return compareNode<DIM_CMP, std::less<>>(min, node);
    }

    /**
     * Find the maximum node on a dimension
     * Time Complexity: ?
     * @tparam DIM_CMP comparison dimension
     * @tparam DIM current dimension of node
     * @param node
     * @return the maximum node on a dimension
     */
    template<size_t DIM_CMP, size_t DIM>
    Node *findMax(Node *node) {
        constexpr size_t DIM_NEXT = (DIM + 1) % KeySize;
        // TODO: implement this function
        if (!node) return nullptr;
        Node* max = findMax<DIM_CMP, DIM_NEXT>(node->right);
        if (DIM_CMP!=DIM) {
            Node* leftMax = findMax<DIM_CMP, DIM_NEXT>(node->left);
            max = compareNode<DIM_CMP, std::greater<>>(max, leftMax);
        }
        return compareNode<DIM_CMP, std::greater<>>(max, node);
    }

    template<size_t DIM>
    Node *findMinDynamic(size_t dim) {
        constexpr size_t DIM_NEXT = (DIM + 1) % KeySize;
        if (dim >= KeySize) {
            dim %= KeySize;
        }
        if (dim == DIM) return findMin<DIM, 0>(root);
        return findMinDynamic<DIM_NEXT>(dim);
    }

    template<size_t DIM>
    Node *findMaxDynamic(size_t dim) {
        constexpr size_t DIM_NEXT = (DIM + 1) % KeySize;
        if (dim >= KeySize) {
            dim %= KeySize;
        }
        if (dim == DIM) return findMax<DIM, 0>(root);
        return findMaxDynamic<DIM_NEXT>(dim);
    }

    /**
     * Erase a node with key (check the pseudocode in project description)
     * Time Complexity: max{O(k log n), O(findMin)}
     * @tparam DIM current dimension of node
     * @param node
     * @param key
     * @return nullptr if node is erased, else the (probably) replaced node
     */
    template<size_t DIM>
    Node *erase(Node *node, const Key &key) {
        constexpr size_t DIM_NEXT = (DIM + 1) % KeySize;
        // TODO: implement this function
        if (node==nullptr) {
            return nullptr;
        }
        if (key==node->key()) {
            if ( (node->left==nullptr)&&(node->right==nullptr) ) {
                treeSize--;
                delete node;
                return nullptr;
            } else if (node->right) {
                Node* rightMin = findMin<DIM, DIM_NEXT>(node->right);
                // node->key() = rightMin->key();
                auto &temp = const_cast<Key &>(node->key());
                temp = rightMin->key();
                node->value() = rightMin->value();
                node->right = erase<DIM_NEXT>(node->right, temp);
            } else if (node->left) {
                Node* leftMax = findMax<DIM, DIM_NEXT>(node->left);
                // node->key() = leftMax->key();
                auto &temp = const_cast<Key &>(node->key());
                temp = leftMax->key();
                node->value() = leftMax->value();
                node->left = erase<DIM_NEXT>(node->left, temp);
            }
        } else {
            if (std::get<DIM>(key)<std::get<DIM>(node->key())) {
                node->left = erase<DIM_NEXT>(node->left, key);
            } else {
                node->right = erase<DIM_NEXT>(node->right, key);
            }
        }
        return node;
    }

    template<size_t DIM>
    Node *eraseDynamic(Node *node, size_t dim) {
        constexpr size_t DIM_NEXT = (DIM + 1) % KeySize;
        if (dim >= KeySize) {
            dim %= KeySize;
        }
        if (dim == DIM) return erase<DIM>(node, node->key());
        return eraseDynamic<DIM_NEXT>(node, dim);
    }

    // TODO: define your helper functions here if necessary
    void copyFrom(const KDTree &that) {
        if (&that==this) return;
        else {
            deleteAll();
            root = copyFromHelper(that.root, nullptr);
            treeSize = that.treeSize;
        }
    }

    Node* copyFromHelper(Node* thatroot, Node* parent) {
        if (!thatroot) return nullptr;
        Node* temp = new Node(thatroot->key(), thatroot->value(), parent);
        temp->left = copyFromHelper(thatroot->left, temp);
        temp->right = copyFromHelper(thatroot->right, temp);
        return temp;
    }

    void deleteAll(){
        deleteNode(root);
    }

    void deleteNode (Node* node) {
        if (!node) return;
        deleteNode(node->left);
        deleteNode(node->right);
        delete node;
    }

    template <size_t DIM>
    Node *initial(std::vector<std::pair<Key, Value>> &v, int left, int right, Node *parent) {
        // std::cout << left << ' ' << right << std::endl;
        constexpr size_t DIM_NEXT = (DIM + 1) % KeySize;
        if (left > right)
        {
            return nullptr;
        }
        int mid = left + (right - left) / 2;
        std::nth_element(v.begin() + left, v.begin() + mid, v.begin() + right + 1, [](const std::pair<Key, Value> &lhs, const std::pair<Key, Value> &rhs) {
            return compareKey<DIM, std::less<>>(lhs.first, rhs.first);
        });
        auto median = v[mid];
        Node *node = new Node{median.first, median.second, parent};
        node->left = initial<DIM_NEXT>(v, left, mid - 1, node);
        node->right = initial<DIM_NEXT>(v, mid + 1, right, node);
        return node;
    }

public:
    KDTree() = default;

    /**
     * Time complexity: O(kn log n)
     * @param v we pass by value here because v need to be modified
     */
    explicit KDTree(std::vector<std::pair<Key, Value>> v) {
        // TODO: implement this function
        root = nullptr;
        treeSize = v.size();
        if (v.empty()) return;
        std::stable_sort(v.begin(), v.end());
        auto last = std::unique(v.rbegin(), v.rend(), [](const std::pair<Key, Value> &lhs, const std::pair<Key, Value> &rhs) {
            return lhs.first == rhs.first;
        });
        v.erase(v.begin(), last.base());
        this->root = initial<0>(v, 0, static_cast<int>(v.size()) - 1, nullptr);
    }

    /**
     * Time complexity: O(n)
     */
    KDTree(const KDTree &that) {
        // TODO: implement this function
        copyFrom(that);
    }

    /**
     * Time complexity: O(n)
     */
    KDTree &operator=(const KDTree &that) {
        // TODO: implement this function
        copyFrom(that);
        return *this;
    }

    /**
     * Time complexity: O(n)
     */
    ~KDTree() {
        // TODO: implement this function
        deleteAll();
    }

    Iterator begin() {
        if (!root) return end();
        auto node = root;
        while (node->left) node = node->left;
        return Iterator(this, node);
    }

    Iterator end() {
        return Iterator(this, nullptr);
    }

    Iterator find(const Key &key) {
        return Iterator(this, find<0>(key, root));
    }

    void insert(const Key &key, const Value &value) {
        insert<0>(key, value, root, nullptr);
    }

    template<size_t DIM>
    Iterator findMin() {
        return Iterator(this, findMin<DIM, 0>(root));
    }

    Iterator findMin(size_t dim) {
        return Iterator(this, findMinDynamic<0>(dim));
    }

    template<size_t DIM>
    Iterator findMax() {
        return Iterator(this, findMax<DIM, 0>(root));
    }

    Iterator findMax(size_t dim) {
        return Iterator(this, findMaxDynamic<0>(dim));
    }

    bool erase(const Key &key) {
        auto prevSize = treeSize;
        erase<0>(root, key);
        return prevSize > treeSize;
    }

    Iterator erase(Iterator it) {
        if (it == end()) return it;
        auto node = it.node;
        if (!it.node->left && !it.node->right) {
            it.node = it.node->parent;
        }
        size_t depth = 0;
        auto temp = node->parent;
        while (temp) {
            temp = temp->parent;
            ++depth;
        }
        eraseDynamic<0>(node, depth % KeySize);
        return it;
    }

    size_t size() const { return treeSize; }
};
