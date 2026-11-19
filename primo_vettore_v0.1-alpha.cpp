#include <iostream>
#include <vector>
#include <limits>
using namespace std;

int main() {
    int n;
    cout << "Quanti valori vuoi inserire? ";
    cin >> n;
    if (n <= 0) { cout << "Inserisci almeno un valore.\n"; return 1; }

    vector<int> v(n);
    for (int i = 0; i < n; i++) {
        cout << "Inserisci il valore " << i + 1 << ": ";
        cin >> v[i];
    }

    int somma = 0, massimo = v[0], minimo = v[0];
    for (int i = 0; i < n; i++) {
        somma += v[i];
        if (v[i] > massimo) massimo = v[i];
        if (v[i] < minimo) minimo = v[i];
    }

    float media = static_cast<float>(somma) / n;
    cout << "\nLa media e': " << media;
    cout << "\nLa somma e': " << somma;
    cout << "\nIl massimo e': " << massimo;
    cout << "\nIl minimo e': " << minimo << endl;

    cout << "\nPremi Invio per chiudere...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();
    return 0;
}
