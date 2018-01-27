# Rhythm generator

Program write to FIFO file rhythm from config file.
Rhythm is started with signal USR1 and USR2.
Write to FIFO is asynchronously  between signals but same signals (e.g. send 2 times USR1) is synchronous. 
Program use self-pipe trick (I use two pipe to save asynchronous between signals) and timers for sending signal to write into FIFO

![rhythm_generator_self-pipe_2](https://github.com/Dyzio18/Linux_programming/blob/master/rhythm_generator_self-pipe/live_rhythm_2.gif?raw=true)

![rhythm_generator_self-pipe_1](https://github.com/Dyzio18/Linux_programming/blob/master/rhythm_generator_self-pipe/live_rhythm_1.gif?raw=true)

## Get started

## Compile
`gcc -Wall main.c -lrt -o out.a`

## Run
```
Rhythm generator - run program with this option:
 -d, --delay <float>	set your rhythm delay in milisecond
			(default is 1000 milisecond = 1 sec.)
 -f,--file <path> <path> ...	 set paths to your fifo files.
 -h, --help		 display help.
 -p, --pause		 send pause to fifo 
 -t,--text		 print config file.
 example:
	./rhythm.a -d 1.5 --file fifo1 fifo2 fifo3
```

####Helpers: 

Create fifo:

`mkfifo fifo_a fifo_b fifo_c`

Send signals:

`kill -USR1 $( ps -ef | grep out.a | grep -v grep | awk '{print $2}')`

`kill -USR2 $( ps -ef | grep out.a | grep -v grep | awk '{print $2}')`

 ---
# Description (PL)

## Opis programu:
Program na początku za pomocą getopt_long pobiera z argumentów uruchomieniowych programu odpowiednie parametry.

Użytkownik może podad takie parametry jak:

-h, --help - Wyswietlenie pomocy i parametrów programu.

-t, --text - Flaga wlączająca wypisanie pliku konfiguracyjnego razem z komentarzami.

-p, --pause - Flaga włączająca wypisywanie pauz - kropek '.' do pliku FIFO

-d, --delay <float> - Opoznienie/takt wpisywania znaków do pliku FIFO

-f, --file <path> <path> ... - Ścieżki do plików FIFO

## Sposób działania
Program wczytuje plik konfiguracyjny (rhythm.config) następnie filtruje komentarze.

Czeka na sygnał od użytkownika.
Sygnał USR1 lub USR2 wywołują handler który do odpowiedniego self-pipe wysyła dane.

Tworze 2 self-pipe dla każdego z sygnałów by operacje między nimi były asynchroniczne,
Jest to kolejka FIFO wiec dane z jednego sygnału bedą kolejkowane i synchroniczne (wykonywane jedne po drugim). Natomiast wykonywanie sygnałów USR1 i USR2 będzie asynchroniczne względem siebie.

Do self-pipe wysyłana jest ramka danych – numer lini oraz znak do wypisania.
W głównej petli programu nasłuchuje czy mam dane w self-pipe (za pomocą flag).
Następnie jesli mam dane w self-pipe i skooczyłem wypisywanie poprzedniego rytmu
zdejmuje z potoku porcję danych i uruchamiam timer.

Timer co "delay" wywołuje sygnał który odpowiada za wypisanie odpowiedniego znaku do FIFO.
Jeśli podczas obsługi wypisywania odczytny zostanie koniec rytmu,
timer danego kanału dla sygnału USR1 lub USR2 zostanie zatrzymany.

Dodatkowo program przechwytuje sygnał SIGPIPE, w przypadku błędu wpisywania do FIFO lub zamknięcia FIFO.
Wówczas szukany jest nowy kanał do wypisania, następnie po znalezieniu kanału wysyłany jest sygnał kontynuacji przerwanego wypisywania (kontunuuje wypisywanie tego samego rytmu).