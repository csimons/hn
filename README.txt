hn: HN Reader

    Invoked with no arguments this program will fetch the latest HN
    headlines and display them as numbered lines.  Invoking the program
    subsequently with one or more title numbers will cause the
    associated stories to be opened in the system's default web browser.

    Sample session:

        $ hn
         1 first story
         2 second story
         3 third story
        $ hn 1 3 # open first and third stories


DEPENDENCIES

    - libcurl
    - libxml2


BUILDING

    $ make # final binary is written to bin/hn


INSTALLING

    $ sudo make install

