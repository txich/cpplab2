Kolekcje nazwanych grafów rekurencyjnych
Wstęp

Biblioteka standardowa języka C++ udostępnia bardzo przydatne kontenery, których nie ma w bibliotece C. Język C++ umożliwia definiowanie skomplikowanych struktur danych, na przykład

unordered_map<long, map<string, map<int, set<pair<int, int>>>>>

Często potrzebujemy łączyć kod w C z kodem w C++.

Celem tego zadania jest napisanie w C++ modułu obsługującego kolekcje nazwanych grafów rekurencyjnych, aby można ich używać w C. Moduł składa się z pliku nagłówkowego (z rozszerzeniem .h) i pliku z implementacją (z rozszerzeniem .cpp).

Rozwiązując to zadanie, studenci powinni poznać:

    kolejne kontenery z STL,
    sposób łączenia kodu w C++ z kodem w C,
    metody inicjowania obiektów globalnych w C++ i wynikające stąd problemy,
    przestrzenie nazw,
    preprocesor, elementy kompilowania warunkowego.

Grafy tworzone rekurencyjnie

Nieskierowany graf prosty jest parą składającą się ze zbioru wierzchołków oraz zbioru krawędzi, czyli zbioru dwuelementowych podzbiorów zbioru wierzchołków. Krawędź łączącą wierzchołki x i y zapisujemy jako {x, y}. Wierzchołki są numerowane nieujemnymi liczbami całkowitymi. Nowy nieskierowany graf prosty, nazywany grafem wynikowym tworzymy, zastępując w nieskierowanym grafie prostym, nazywanym biorcą, wybraną krawędź nieskierowanym grafem prostym, nazywanym dawcą. Zakładamy, że biorca i dawca mają co najmniej po dwa wierzchołki. Niech {w_1, w_2} będzie zastępowaną krawędzią biorcy. Niech k będzie liczbą wierzchołków dawcy. Niech v_1 i v_2 oznaczają wybrane różne wierzchołki dawcy. Niech v_3, ..., v_k oznaczają pozostałe wierzchołki dawcy w kolejności rosnącej ich numerów. Graf wynikowy powstaje jako rezultat następującej procedury:

    Tworzymy graf wynikowy jako kopię biorcy. Nie modyfikujemy biorcy.
    Usuwamy z grafu wynikowego krawędź {w_1, w_2}.
    Utożsamiamy wierzchołki w_1 i w_2 grafu wynikowego z wierzchołkami v_1 i v_2 dawcy.
    Dodajemy do grafu wynikowego k - 2 nowych wierzchołków o numerach w_3, ..., w_k, które są kolejnymi najmniejszymi numerami wierzchołków niewystępujących w grafie wynikowym.
    Dla każdej krawędzi {v_i, v_j} w dawcy dodajemy krawędź {w_i, w_j} w grafie wynikowym.

Dawca może być tym samym grafem co biorca.

Każda krawędź ma przypisany poziom – liczbę całkowitą od 0 do LEVEL_MAX - 1. Poziom grafu jest to największy z poziomów jego krawędzi, bądź 0, jeżeli graf nie ma krawędzi. Krawędzie dodane w wyniku zastępowania krawędzi otrzymują poziom równy max(poziom dawcy, poziom zastępowanej krawędzi) + 1.

Grafy mają nazwy. Nazwa grafu jest niepustym napisem składającym się z małych i dużych liter alfabetu angielskiego, cyfr oraz znaku podkreślenia _. Grafy w obrębie kolekcji porządkujemy leksykograficznie według ich nazw. Ten porządek leksykograficzny jest indukowany przez porządek na wartościach ASCII znaków.
Polecenie

Zaimplementuj moduł recursive_graph_collections obsługujący kolekcje tworzonych rekurencyjnie nieskierowanych grafów prostych.

Wymieniona powyżej stała LEVEL_MAX jest liczbą dodatnią ustalaną w trakcie kompilowania modułu. Jej domyślna wartość to 4 i można ją zmieniać parametrem -D kompilatora.

Moduł powinien udostępniać następujące funkcje.

long rgc_new_collection();

Tworzy nową, pustą kolekcję nazwanych grafów rekurencyjnych. Wynikiem jest identyfikator tej kolekcji. Każde wywołanie zwraca nowy identyfikator, począwszy od 0, a skończywszy na LONG_MAX. Jeśli wyczerpią się identyfikatory, zwraca -1.

void rgc_delete_collection(long id);

Jeżeli istnieje kolekcja nazwanych grafów rekurencyjnych o identyfikatorze id, usuwa ją, a w przeciwnym przypadku niczego nie robi.

bool rgc_new_graph(long id, char const *name, int n);

Jeśli istnieje kolekcja o identyfikatorze id, wskaźnik name jest różny od nullptr i wskazuje na poprawną nazwę grafu niewystępującą w kolekcji, wartość n jest nieujemna, tworzy w tej kolekcji nowy graf o podanej nazwie, n wierzchołkach i pustym zbiorze krawędzi. Jeśli n jest dodatnie, to wierzchołki są numerowane od 0 do n - 1. Wynikiem funkcji jest true, jeśli graf został utworzony, a false w przeciwnym przypadku.

void rgc_delete_graph(long id, char const *name);

Jeśli istnieje kolekcja o identyfikatorze id, wskaźnik name jest różny od nullptr i w kolekcji jest graf o nazwie name, usuwa go, a w przeciwnym przypadku niczego nie robi.

int rgc_graph_new_node(long id, char const *name);

Jeśli istnieje kolekcja o identyfikatorze id, wskaźnik name jest różny od nullptr i w kolekcji jest graf o nazwie name o mniej niż INT_MAX wierzchołkach, dodaje do niego nowy, izolowany wierzchołek. Nowy wierzchołek otrzymuje najmniejszy nieujemny numer niewystępujący w grafie. Wynikiem funkcji jest numer nowego wierzchołka, jeśli został utworzony, a -1 w przeciwnym przypadku.

bool rgc_graph_delete_node(long id, char const *name, int x);

Jeśli istnieje kolekcja o identyfikatorze id, wskaźnik name jest różny od nullptr i w kolekcji jest graf o nazwie name mający wierzchołek o numerze x, usuwa ten wierzchołek wraz ze wszystkimi krawędziami, do których należy. Numery pozostałych wierzchołków się nie zmieniają. Wynikiem funkcji jest true, jeśli wierzchołek został usunięty, a false w przeciwnym przypadku.

bool rgc_graph_add_edge(long id, char const *name, int x, int y);

Jeśli istnieje kolekcja o identyfikatorze id, wskaźnik name jest różny od nullptr i w kolekcji jest graf o nazwie name z różnymi wierzchołkami o numerach x i y niepołączonymi krawędzią, dodaje do grafu krawędź {x, y} z poziomem 0. Wynikiem funkcji jest true, jeśli krawędź została dodana, a false w przeciwnym przypadku.

bool rgc_graph_delete_edge(long id, char const *name, int x, int y);

Jeśli istnieje kolekcja o identyfikatorze id, wskaźnik name jest różny od nullptr i w kolekcji jest graf o nazwie name mający krawędź {x, y}, usuwa tę krawędź. Wynikiem funkcji jest true, jeśli krawędź została usunięta, a false w przeciwnym przypadku.

char const * rgc_first_graph(long id);

Jeśli istnieje niepusta kolekcja o identyfikatorze id, wynikiem funkcji jest wskaźnik na nazwę pierwszego grafu w tej kolekcji, a nullptr w przeciwnym przypadku.

char const * rgc_next_graph(long id, char const *name);

Jeśli istnieje kolekcja o identyfikatorze id, wskaźnik name jest różny od nullptr i w kolekcji istnieje graf następny w kolejności po grafie o nazwie name, wynikiem funkcji jest wskaźnik na nazwę tego grafu, a nullptr w przeciwnym przypadku.

int rgc_first_node(long id, char const *name);

Jeśli istnieje kolekcja o identyfikatorze id, wskaźnik name jest różny od nullptr i w kolekcji jest graf o nazwie name mający co najmniej jeden wierzchołek, wynikiem funkcji jest najmniejszy występujący w nim numer wierzchołka, a -1 w przeciwnym przypadku.

int rgc_next_node(long id, char const *name, int x);

Jeśli istnieje kolekcja o identyfikatorze id, wskaźnik name jest różny od nullptr i w kolekcji jest graf o nazwie name, który ma wierzchołek o numerze większym niż x, wynikiem funkcji jest najmniejszy numer takiego wierzchołka, a -1 w przeciwnym przypadku.

int rgc_first_edge(long id, char const *name, int x);

Jeśli istnieje kolekcja o identyfikatorze id, wskaźnik name jest różny od nullptr i w kolekcji jest graf o nazwie name, w którym występuje wierzchołek o numerze x mający sąsiada, wynikiem funkcji jest najmniejszy numer sąsiada tego wierzchołka, a -1 w przeciwnym przypadku.

int rgc_next_edge(long id, char const *name, int x, int y);

Jeśli istnieje kolekcja o identyfikatorze id, wskaźnik name jest różny od nullptr i w kolekcji jest graf o nazwie name, w którym występuje wierzchołek o numerze x mający sąsiada o numerze większym niż y, wynikiem funkcji jest najmniejszy większy od y numer sąsiada wierzchołka x, a -1 w przeciwnym przypadku.

bool rgc_replace_edge(long id, char const *result_name, char const *recipient_name, int w_1, int w_2, char const *donor_name, int v_1, int v_2);

Jeśli istnieje kolekcja o identyfikatorze id, wskaźniki result_name, recipient_name i donor_name są różne od nullptr, result_name wskazuje na poprawną nazwę grafu i grafu o takiej nazwie nie ma w kolekcji, w kolekcji jest biorca o nazwie recipient_name, w kolekcji jest dawca o nazwie donor_name, biorca ma krawędź {w_1, w_2}, dawca ma dwa różne wierzchołki v_1 i v_2, poziom grafu wynikowego będzie mniejszy niż LEVEL_MAX, liczba wierzchołków grafu wynikowego nie przekroczy INT_MAX, tworzy w kolekcji graf wynikowy o nazwie result_name powstały za pomocą opisanej wyżej procedury zastępowania. Wynikiem funkcji jest true, jeśli graf został utworzony, a false w przeciwnym przypadku.

ssize_t rgc_edge_level(long id, char const *name, int x, int y);

Jeśli istnieje kolekcja o identyfikatorze id, wskaźnik name jest różny od nullptr i w kolekcji jest graf o nazwie name mający krawędź {x, y}, wynikiem funkcji jest poziom tej krawędzi, a -1 w przeciwnym przypadku.

ssize_t rgc_graph_level(long id, char const *name);

Jeśli istnieje kolekcja o identyfikatorze id, wskaźnik name jest różny od nullptr i w kolekcji jest graf o nazwie name, wynikiem funkcji jest poziom tego grafu, a -1 w przeciwnym przypadku.

ssize_t rgc_max_level();

Wynikiem funkcji jest maksymalny dopuszczalny poziom krawędzi, czyli LEVEL_MAX - 1.

ssize_t rgc_count_nodes(long id, char const *name);

Jeśli istnieje kolekcja o identyfikatorze id, wskaźnik name jest różny od nullptr i w kolekcji jest graf o nazwie name, wynikiem funkcji jest liczba wierzchołków tego grafu, a -1 w przeciwnym przypadku.

ssize_t rgc_count_edges(long id, char const *name);

Jeśli istnieje kolekcja o identyfikatorze id, wskaźnik name jest różny od nullptr i w kolekcji jest graf o nazwie name, wynikiem funkcji jest liczba krawędzi tego grafu, a -1 w przeciwnym przypadku.

ssize_t rgc_size(long id);

Jeśli istnieje kolekcja o identyfikatorze id, wynikiem funkcji jest liczba grafów w tej kolekcji, a -1 w przeciwnym przypadku.
Wymagania formalne

Oczekiwane rozwiązanie powinno korzystać z kontenerów i metod udostępnianych przez standardową bibliotekę C++. Nie należy definiować własnych struktur, unii lub klas, a przynajmniej takich, które zawierają dane.

Kolekcje powinny przechowywać kopie nazw, a nie wartości przekazanych wskaźników.

Powinna być możliwość używania wyżej opisanych funkcji w języku C++. Przy kompilowaniu pliku nagłówkowego modułu w C++ deklaracje funkcji powinny znaleźć się w przestrzeni nazw cxx.

Należy ukryć przed światem zewnętrznym wszystkie zmienne globalne i funkcje pomocnicze nienależące do interfejsu modułu.

Wolno założyć, że nie zabraknie pamięci. Nie wymagamy obsługiwania wyjątku braku pamięci.

Rozwiązanie powinno składać się z plików recursive_graph_collections.h i recursive_graph_collections.cpp, które należy umieścić w Moodle.

Rozwiązanie będzie kompilowane i testowane na maszynie students.
Przykłady użycia

Przykłady użycia znajdują się w poniżej załączonych plikach recursive_graph_collections_example_1.c i recursive_graph_collections_example_2.cpp. Kompilowanie przykładów przy założeniu, że znajdują się w bieżącym katalogu, a rozwiązanie jest w katalogu ../rozwiązanie:

g++ -c -Wall -Wextra -O2 -std=c++23 ../rozwiazanie/recursive_graph_collections.cpp -o recursive_graph_collections_4.o
g++ -c -Wall -Wextra -O2 -std=c++23 -DLEVEL_MAX=16 ../rozwiazanie/recursive_graph_collections.cpp -o recursive_graph_collections_16.o
gcc -c -Wall -Wextra -O2 -std=c23 -I../rozwiazanie recursive_graph_collections_example_1.c -o recursive_graph_collections_example_1.o
g++ -c -Wall -Wextra -O2 -std=c++23 -I../rozwiazanie recursive_graph_collections_example_2.cpp -o recursive_graph_collections_example_2.o
g++ recursive_graph_collections_example_1.o recursive_graph_collections_4.o -o recursive_graph_collections_example_1
g++ recursive_graph_collections_example_2.o recursive_graph_collections_16.o -o recursive_graph_collections_example_2_a
g++ recursive_graph_collections_16.o recursive_graph_collections_example_2.o -o recursive_graph_collections_example_2_b

Ocenianie rozwiązania
Ocena automatyczna

Za testy automatyczne zostanie przyznana ocena z przedziału od 0 do 6 punktów. Za błędną nazwę pliku zostanie odjęty 1 punkt. Za ostrzeżenia wypisywane przez kompilator zostanie odjęty 1 punkt. Nie ma punktów ułamkowych.
Ocena jakości kodu

Ocena jakości kodu jest z przedziału od 0 do 4 punktów. Nie ma punktów ułamkowych. Odejmujemy punkty za:

    brzydki styl (niepoprawne wstawianie spacji, wcięć, odstępów, brak komentarzy, magiczne stałe itd.);
    dłubanie własnych klas, struktur lub algorytmów zamiast użycia STL-owych;
    zły dobór typu lub kontenera, brak nazw typów, niewiele mówiące nazwy typów;
    rozwlekłą lub nieelegancką strukturę programu, rozpatrywanie zbyt wielu warunków brzegowych, powtarzanie kodu, nieefektywne korzystanie z klasy string, np. if (str != "") zamiast if (!str.empty()), przechowywanie liczb jako napisów;
    wprowadzanie sztucznych ograniczeń na rozmiar danych;
    nieusuwanie lub nieefektywne usuwanie niepotrzebnych już danych;
    nieskuteczną obsługę (lub jej brak) problemu „static initialization order fiasco” (czytanka „Inicjowanie obiektów globalnych”), o ile nie zostanie to wykryte przez testy automatyczne; inicjowanie strumienia diagnostycznego w każdej funkcji (przez powielenie kodu inicjującego);
    potencjalne wycieki pamięci albo korzystanie z new i delete;
    niepoprawne pliki nagłówkowe, brak include guard #ifndef #define #endif, brak #ifdef __cplusplus, brakujące #include, zbędne #include;
    nieukrycie przed światem zewnętrznym wszystkich zmiennych globalnych i funkcji pomocniczych nienależących do interfejsu modułu;
    użycie typedef zamiast using;
    błędy w stosowaniu przestrzeni nazw;
    wielokrotne wyszukiwanie tego samego klucza w mapie, np. za pomocą operatora [];
    inne znalezione i niewymienione w powyższych kryteriach błędy niewykryte przez testy automatyczne.

Ponadto piętnujemy:

    przekazywanie funkcjom dużych argumentów przez wartość – takie obiekty przekazujemy przez stałą referencję, czyli const &; na razie tylko wskazujemy te błędy i nie odejmujemy za nie punktów, bo są to zagadnienia pojawiające się w kolejnych zadaniach, w których już będziemy za to karać.

