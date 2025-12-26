// RUN: %check_clang_tidy -std=c++17-or-later %s bugprone-optional-access %t -- --fix-notes
// RUN: %check_clang_tidy -check-suffix=CUSTOM -std=c++17-or-later %s bugprone-optional-access %t -- \
// RUN: -config="{CheckOptions: {bugprone-optional-access.OptionalTypes: 'CustomOptional'}}" --fix-notes

struct Foo {
    int x;
};

namespace std {
  template<typename T>
  struct optional
  {
    constexpr optional() noexcept;
    constexpr optional(T&&) noexcept;
    constexpr optional(const T&) noexcept;
    template<typename U>
    constexpr optional(U&&) noexcept;
    const T& operator*() const;
    T* operator->();
    const T* operator->() const;
    T& operator*();
    const T& value() const;
    T& value();
    const T& get() const;
    T& get();
    T value_or(T) const;

    explicit operator bool() const;
  };

  template <class T>
  T&& move(T &x) {
    return static_cast<T&&>(x);
  }
}

namespace boost {
  template<typename T>
  struct optional {
    constexpr optional() noexcept;
    constexpr optional(const T&) noexcept;
    const T& operator*() const;
    const T* operator->() const;
    const T& get() const;

    explicit operator bool() const;
  };
}

namespace absl {
  template<typename T>
  struct optional {
    constexpr optional() noexcept;
    constexpr optional(const T&) noexcept;
    const T& operator*() const;
    const T* operator->() const;
    const T& value() const;

    explicit operator bool() const;
  };
}

template<typename T>
struct CustomOptional {
  CustomOptional();
  CustomOptional(const T&);
  const T& Read() const;
  T& operator*();
  T* operator->();
  T& Ooo();

  explicit operator bool() const;
};

void testStarAccess() {
    std::optional<int> a;
    boost::optional<int> b;
    absl::optional<int> c;

    int x;

    x = *a;
    // CHECK-MESSAGES: :[[@LINE-1]]:9: warning: remove error-prone optional access (via operator '*') to 'std::optional<int>' [bugprone-optional-access]
    // CHECK-FIXES: x = *a;

    x = *b;
    // CHECK-MESSAGES: :[[@LINE-1]]:9: warning: remove error-prone optional access (via operator '*') to 'boost::optional<int>' [bugprone-optional-access]
    // CHECK-FIXES: x = *b;

    x = *c;
    // CHECK-MESSAGES: :[[@LINE-1]]:9: warning: remove error-prone optional access (via operator '*') to 'absl::optional<int>' [bugprone-optional-access]
    // CHECK-FIXES: x = *c;    
}

void testCustomStarAccess() {
  CustomOptional<int> d;

  int x = *d;
  // CHECK-MESSAGES-CUSTOM: :[[@LINE-1]]:11: warning: remove error-prone optional access (via operator '*') to 'CustomOptional<int>' [bugprone-optional-access]
  // CHECK-FIXES-CUSTOM: int x = *d;
}

void testArrowAccess() {
    std::optional<Foo> a;
    boost::optional<Foo> b;
    absl::optional<Foo> c;

    int x;

    x = a->x;
    // CHECK-MESSAGES: :[[@LINE-1]]:9: warning: remove error-prone optional access (via operator '->') to 'std::optional<Foo>' [bugprone-optional-access]
    // CHECK-FIXES: x = a->x;

    x = b->x;
    // CHECK-MESSAGES: :[[@LINE-1]]:9: warning: remove error-prone optional access (via operator '->') to 'boost::optional<Foo>' [bugprone-optional-access]
    // CHECK-FIXES: x = b->x;

    x = c->x;
    // CHECK-MESSAGES: :[[@LINE-1]]:9: warning: remove error-prone optional access (via operator '->') to 'absl::optional<Foo>' [bugprone-optional-access]
    // CHECK-FIXES: x = c->x;    
}

void testCustomArrowAccess() {
  CustomOptional<Foo> d;

  int x = d->x;
  // CHECK-MESSAGES-CUSTOM: :[[@LINE-1]]:11: warning: remove error-prone optional access (via operator '->') to 'CustomOptional<Foo>' [bugprone-optional-access]
  // CHECK-FIXES-CUSTOM: int x = d->x;
}

void testExplicitBoolConversion() {
  std::optional<int> x;

  if (x) {
    // CHECK-MESSAGES: :[[@LINE-1]]:7: warning: remove error-prone implicit convertion from 'std::optional<int>' to bool [bugprone-optional-access]
    // CHECK-FIXES: if (x) { 
    int y = x.value();
  }

  if (!x) {
    // CHECK-MESSAGES: :[[@LINE-1]]:8: warning: remove error-prone implicit convertion from 'std::optional<int>' to bool [bugprone-optional-access]
    // CHECK-FIXES: if (!x) { 
    int y = x.value();
  }
}

void testCustomExplicitBoolConversion() {
  CustomOptional<int> x;

  if ((x)) {
    // CHECK-MESSAGES-CUSTOM: :[[@LINE-1]]:7: warning: remove error-prone implicit convertion from 'CustomOptional<int>' to bool [bugprone-optional-access]
    // CHECK-FIXES-CUSTOM: if (x) { 
    int y = x.Read();
  }

  if (!(x)) {
    // CHECK-MESSAGES-CUSTOM: :[[@LINE-1]]:8: warning: remove error-prone implicit convertion from 'CustomOptional<int>' to bool [bugprone-optional-access]
    // CHECK-FIXES-CUSTOM: if (!x) { 
    int y = x.Read();
  }
}