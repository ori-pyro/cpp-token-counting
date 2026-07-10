# cpp-token-counter

Данная программа принимает папку проекта или ссылку на Github репозиторий и показывает численные характеристики проекта такие как:

* comment (Число комменатирев в коде)
* file (Число файлов)
* identifier (Число идентификаторов)
* keyword (Число ключевых слов из стандарта c++, которые встречаются в коде. Полный список ключевых слов, которые различает программа приведён ниже)
* line (Число строк. Многострочные комментарии и пустые строки не учитываются при подсчёте)
* literal (Общее число литералов. Также есть количество литералов каждого типа)
* operator-or-punctuator (Число всех возможных операторов и пунктуаторов. Полный список операторов и пунктуаторов, которые различает программа приведён ниже)

Программа поддерживает проверку файлов, с данными расширениями:

* .cpp
* .cc
* .cxx
* .h
* .hpp
* .hh
* .hxx

Программа НЕ различает код на с++ и c в .h файлах.

Основные ограничения и замечания:

* Программа не считает директивы препроцессора.
* Программа не отличает треугольные скобки и знаки неравенств друг от друга.
* Программа считает все виды скобок за одну лексему. (Считает только число открывающихся скобок)

Список ключевых слов:
* bool
* char
* char8_t
* char16_t
* char32_t
* double
* float
* int
* long
* short
* signed
* unsigned
* void
* wchar_t
* false
* true
* typedef
* break
* case
* continue
* default
* do
* else
* for
* goto
* if
* return
* switch
* while
* new
* delete
* class
* struct
* union
* enum
* public
* private
* protected
* virtual
* friend
* this
* mutable
* explicit
* operator
* catch
* throw
* try
* template
* typename
* export
* namespace
* using
* const_cast
* dynamic_cast
* reinterpret_cast
* static_cast
* const
* volatile
* static
* extern
* auto
* register
* thread_local
* alignas
* alignof
* decltype
* typeid
* sizeof
* static_assert
* noexcept
* concept
* requires
* constexpr
* consteval
* constinit
* nullptr
* co_await
* co_return
* co_yield
* or
* and
* xor
* not
* bitand
* bitor
* compl
* and_eq
* or_eq
* xor_eq
* not_eq
* asm
* atomic_cancel
* atomic_commit
* atomic_noexcept
* contract_asset
* reflexpr
* synchronized
* inline

Список операторов:
* \+
* \-
* \*
* \/
* \%
* \+\+
* \-\-
* \=\=
* \!\=
* \<
* \>
* \<\=
* \>\=
* \<\=\>
* \!
* \&\&
* \|\|
* \~
* \&
* \|
* \^
* \<\<
* \>\>
* \=
* \+\=
* \-\=
* \*\=
* \/\=
* \%\=
* \&\=
* \|\=
* \^\=
* \<\<=
* \>\>=
* \.
* \-\>
* \.\*
* \-\>\*
* \:\:
* \?\:
* \,
* \(
* \{
* \[
* \;
* \:
* \#
* \.\-\.
* and
* and_eq
* bitand
* bitor
* compl
* not
* not_eq
* or
* or_eq
* xor
* xor_eq
* \<%
* \<:
* %\:
* %\:%\:

