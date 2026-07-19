#pragma once

#include <typeinfo>
#include <utility>

class Any {
public:
    Any() noexcept : _content(nullptr) {
    }

    template<class T>
    explicit Any(const T &value) : _content(new PlaceHolder<T>{value}) {
    }

    /**
     *
     * @tparam T 如果 T 是 Any 类型，则禁用模板，防止把 Any 对象当作普通值存储，而不是拷贝，让它走 Any(const Any&)
     */
    template<class T, class = std::enable_if_t<!std::is_same_v<std::decay_t<T>, Any>> >
    explicit Any(T &&value)
        : _content(new PlaceHolder<std::decay_t<T> >{std::forward<T>(value)}) {
    }


    Any(const Any &other) : _content(other._content ? other._content->clone() : nullptr) {
    }

    // 移动构造
    Any(Any &&other) noexcept
        : _content(std::exchange(other._content, nullptr)) {
    }

    ~Any() {
        delete _content;
    }

    Any &swap(Any &other) noexcept {
        other._content = std::exchange(_content, other._content);
        return *this;
    }

    template<class T>
    Any &operator=(T &&value) {
        Any(std::forward<T>(value)).swap(*this);
        return *this;
    }

    Any &operator=(const char *value) {
        Any(std::string(value)).swap(*this);
        return *this;
    }

    Any &operator=(const Any &other) {
        if (other._content) {
            Any(other).swap(*this);
        } else {
            reset();
        }
        return *this;
    }

    Any &operator=(Any &&other) noexcept {
        Any(std::move(other)).swap(*this);
        return *this;
    }

    template<class T>
    [[nodiscard]] T *get() {
        return is_type<T>()
                   ? &static_cast<PlaceHolder<T> *>(_content)->_value
                   : nullptr;
    }

    template<class T>
    [[nodiscard]] const T *get() const {
        if (!_content || typeid(T) != _content->type()) {
            return nullptr;
        }
        return &static_cast<const PlaceHolder<T> *>(_content)->_value;
    }

    /**
     *
     * @return 获取值，错误抛出异常
     */
    template<class T>
    [[nodiscard]] T &any_cast() {
        auto *ptr = get<T>();
        if (!ptr) {
            throw std::bad_cast();
        }
        return *ptr;
    }

    /**
     *
     * @return 获取值，错误抛出异常
     */
    template<class T>
    [[nodiscard]] const T &any_cast() const {
        auto *ptr = get<T>();
        if (!ptr) {
            throw std::bad_cast();
        }
        return *ptr;
    }

    [[nodiscard]] bool has_value() const noexcept {
        return _content != nullptr;
    }

    // 重置为空
    void reset() noexcept {
        delete _content;
        _content = nullptr;
    }

    template<class T>
    [[nodiscard]] bool is_type() const {
        return _content && typeid(T) == _content->type();
    }

    [[nodiscard]] const std::type_info &type() const {
        static const auto &void_type = typeid(void);
        return _content ? _content->type() : void_type;
    }

    explicit operator bool() const noexcept {
        return has_value();
    }

private:
    class Holder {
    public:
        virtual ~Holder() = default;

        [[nodiscard]] virtual const std::type_info &type() const = 0;

        [[nodiscard]] virtual Holder *clone() const = 0;
    };

    template<class T>
    class PlaceHolder : public Holder {
    public:
        explicit PlaceHolder(T value) : _value(std::move(value)) {
        }

        [[nodiscard]] const std::type_info &type() const override {
            return typeid(T);
        }

        [[nodiscard]] Holder *clone() const override {
            return new PlaceHolder{_value};
        }

    public:
        T _value;
    };

    Holder *_content;
};

inline void swap(Any &lhs, Any &rhs) noexcept {
    lhs.swap(rhs);
}
