
Zadanie 2
Napisane przez: Marcin Peczarski (piątek, 16 października 2026, 13:48)
Liczba odpowiedzi: 12
W Moodle zostały odsłonięte materiały do zadania 2. Zadania 2 do 6 rozwiązują Państwo w zespołach dobieranych w ramach grupy zajęciowej. Zespoły są zasadniczo dwuosobowe, chyba że do rozwiązywania zadania zgłosi się nieparzysta liczba osób – wtedy dopuszczamy jeden zespół trzyosobowy. Składy zespołów zatwierdza prowadzący grupę. Żeby nie tracić czasu na zajęciach, proszę próbować się dobierać w zespołu przed pierwszymi zajęciami z danego zadania. Przypominam, że składy zespołów nie powinny się powtarzać (zastosowanie ma tu punkt 6 ważnych informacji organizacyjnych).
Obraz Marcin Peczarski
W odpowiedzi na Marcin Peczarski
Odp: Zadanie 2
Napisane przez: Marcin Peczarski (niedziela, 18 października 2026, 21:16)
W Moodle została opublikowana treść zadania 2.
Obraz Michał Kołodziejczyk
W odpowiedzi na Marcin Peczarski
Odp: Zadanie 2
Napisane przez: Michał Kołodziejczyk (środa, 21 października 2026, 09:07)
Dzień dobry,
jak powinno działać nadawanie identyfikatorów kolekcjom? Czy każde wywołanie rgc_new_collection() powinno zawsze zwrócić wartość o 1 większą od poprzedniego wywołania tej funkcji (oprócz pierwszego zwracającego 0, i każdego następnego po LONG_MAX, zwracających -1)? Czy identyfikator "zwolniony" przez rgc_delete_collection() powinien zostać użyty ponownie przy pewnym późniejszym wywołaniu rgc_new_collection()?

t.j. po wykonaniu kodu:
long x = rgc_new_collection();
rgc_delete_collection(x);
long y = rgc_new_collection();
wartość y powinna być równa x, czy x+1 (lub -1 jeśli x == LONG_MAX)?
Obraz Paweł Putra
W odpowiedzi na Michał Kołodziejczyk
Odp: Zadanie 2
Napisane przez: Paweł Putra (środa, 21 października 2026, 18:38)
> Czy każde wywołanie rgc_new_collection() powinno zawsze zwrócić wartość o 1 większą od poprzedniego wywołania tej funkcji (oprócz pierwszego zwracającego 0, i każdego następnego po LONG_MAX, zwracających -1)?

Tak.

"Czy identyfikator "zwolniony" przez rgc_delete_collection() powinien zostać użyty ponownie przy pewnym późniejszym wywołaniu rgc_new_collection()?

Nie.
Obraz Michał Kołodziejczyk
W odpowiedzi na Marcin Peczarski
Odp: Zadanie 2
Napisane przez: Michał Kołodziejczyk (czwartek, 22 października 2026, 22:42)
W wymaganiach formalnych jest napisane, że "Nie należy definiować własnych struktur, unii lub klas, a przynajmniej takich, które zawierają dane."

Czy zatem zabronione jest (lub spowoduje utratę punktów) stworzenie na przykład klasy Graph, zawierającej zbiór wierzchołków i zbiór krawędzi (jako std::set i std::set) oraz metody typu addNode, removeEdge, etc. Czy tego typu klasa liczy się jako "klasa zawierająca dane"? Jeżeli tak, to jakiego typu klasy są dozwolone?
Obraz Paweł Putra
W odpowiedzi na Michał Kołodziejczyk
Odp: Zadanie 2
Napisane przez: Paweł Putra (piątek, 23 października 2026, 11:56)
Tak, jest zabronione definiowanie takiej klasy, unii lub struktury. Dozwolone
jest użycie „enum class” ewentualnie funktora, czyli klasy lub struktury
zawierającej operator(), np. gdy potrzebujemy funkcji haszującej.
Obraz Ignacy Pękała
W odpowiedzi na Marcin Peczarski
Odp: Zadanie 2
Napisane przez: Ignacy Pękała (piątek, 23 października 2026, 15:12)
W treści zadania określono poziom krawędzi "dodanych" w wyniku procedury zastępowania krawędzi. Proszę o informację, które krawędzie są rozumiane jako "dodane":

1) wszystkie krawędzie grafu wynikowego,

2) wszystkie krawędzie których nie było w grafie biorcy (t.j. krawędzie dawcy z wyłączeniem krawędzi { w1, w2 } jeśli takowa należy do grafu biorcy),

3) wszystkie krawędzie z grafu dawcy (t.j włącznie z krawędzią { w1, w2 } jeśli jest ona obecna w grafie wynikowym),
Obraz Paweł Putra
W odpowiedzi na Ignacy Pękała
Odp: Zadanie 2
Napisane przez: Paweł Putra (niedziela, 25 października 2026, 11:25)
Poprawna interpretacja to: 3) wszystkie krawędzie z grafu dawcy (t.j włącznie z krawędzią { w1, w2 } jeśli jest ona obecna w grafie wynikowym).
Obraz Dominika Szczecińska
W odpowiedzi na Marcin Peczarski
Odp: Zadanie 2
Napisane przez: Dominika Szczecińska (wtorek, 27 października 2026, 12:42)
Dzień dobry,
w jaki sposób biblioteka ma obsługiwać niedodatnią wartość zmiennej LEVEL_MAX?
Obraz Paweł Putra
W odpowiedzi na Dominika Szczecińska
Odp: Zadanie 2
Napisane przez: Paweł Putra (wtorek, 27 października 2026, 14:39)
Można założyć, że `LEVEL_MAX` będzie dodatnią liczbą całkowitą w zapisie dziesiętnym i zmieści się w zmiennej typu `ssize_t`.
W szczególności nie będziemy testować zachowania programów na przykładach typu:

    `-DLEVEL_MAX=1e3`
    `-DLEVEL_MAX=-0xDEADBEEF`
    `-DLEVEL_MAX==(std::vector<int>{1,2,3.4}.size())`, itd...

Obraz Wojciech Grzela
W odpowiedzi na Marcin Peczarski
Odp: Zadanie 2
Napisane przez: Wojciech Grzela (środa, 28 października 2026, 11:10)
Czy w przypadku, gdy w kolekcji o numerze id nie istnieje graf o nazwie name, funkcja rgc_next_graph powinna zwrócić nullptr, czy najmniejszą leksykograficznie nazwę grafu obecnego w kolekcji większą od name (lub nullptr, kiedy takiego grafu nie ma w kolekcji)?
Obraz Paweł Putra
W odpowiedzi na Wojciech Grzela
Odp: Zadanie 2
Napisane przez: Paweł Putra (czwartek, 29 października 2026, 10:12)
Najmniejszą leksykograficznie nazwę grafu obecnego w kolekcji większą od name (lub nullptr, kiedy takiego grafu nie ma w kolekcji).
Obraz Michał Pilarski
W odpowiedzi na Marcin Peczarski
Odp: Zadanie 2
Napisane przez: Michał Pilarski (czwartek, 29 października 2026, 09:00)
Czy nazwy grafów w testach będą na tyle krótkie, by można było używać std::regex_match bez obaw o przepełnienie stosu?
