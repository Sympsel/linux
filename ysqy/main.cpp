#include <print>
#include <memory>

struct freeNode {
    int begin;
    int size;
    freeNode *next = nullptr;
};

constexpr int SIZE = 1024;

constexpr unsigned int address = 0xff;

/**
 *
 * 空闲分区表，最坏适应算法
 */
class FreeList {
public:
    FreeList() {
        _list = new freeNode(0, address + 1, nullptr);
    }

    ~FreeList() {
        auto curr = _list;
        while (curr != nullptr) {
            const auto next = curr->next;
            delete curr;
            curr = next;
        }
    }

    void release(int begin, int size) {
        if (_list == nullptr) {
            _list = new freeNode(begin, size, nullptr);
        } else {
            auto curr = _list;
            while (curr->next != nullptr) {
                if (curr->next->size <= size) {
                    const auto newNode = new freeNode(begin, size, curr->next);
                    curr->next = newNode;
                    return;
                }
                curr = curr->next;
            }
        }
    }

    void use(int size) {
        if (_list == nullptr || _list->size < size) {
            std::println("没有足够空间");
            return;
        }
        if (_list->size == size) {
            const auto next = _list->next;
            delete _list;
            _list = next;
        } else {
            _list->begin += size;
            _list->size -= size;
            reorder();
        }
    }

private:
    freeNode *_list;

    void reorder() {
        if (_list == nullptr || _list->next == nullptr || _list->size >= _list->next->size) {
            return;
        }
        auto node = _list;
        _list = _list->next;

        auto curr = _list;
        while (curr->next != nullptr && curr->next->size > node->size) {
            curr = curr->next;
        }
        node->next = curr->next;
        curr->next = node;
    }
};

int main() {
    auto freeList = std::make_unique<FreeList>();
    return 0;
}
