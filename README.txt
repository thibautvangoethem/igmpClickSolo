Voor het programma runnende te krijgen, moet u de igmp directory die staat in click/elements vervangen met de igmp directory van de inzending, hetzelfde moet gedaan worden voor de library folder die staat in click/scripts. Daarna is het nodig om click te builden.

De inzending bevat ook een aparte folder met de naam bash_scripts, deze bevat enkele bash scripts die telnet commando's automatiseren. Deze scripts heb ik gebruikt om mijn implementatie zo gelijkstaand te vergelijking met de referentie implementatie. Verder zijn er ook heel wat scripts om de handlers te testen die ik ook in mijn implementatie heb geïmplementeerd. Deze handlers zijn niet altijd hetzelfde zoals in de referentie oplossing, dus als u handlers wilt testen voor mijn implementatie kan u best de scripts raadplegen.

--EXTRA--
-Alle handlers zijn geïmplementeerd.
-Er is een startup query interval van 1/4 query interval volgens rfc 8.6 .

--Afwijking tegenover de referentie--
-In de referentie implementatie is er random interval tussen specific queries dat in [0,lmqi] ligt, Het blijkt ook dat de specific queries verzonden worden als antwoord op de (redundant)leave membership reports. Volgens rfc 6.6.3.1 (of 6.4.2) staat echter dat alleen de eerste specific query een direct antwoord moet zijn op op de eerste leave, en dat alle lmqc-1 specific queries erachter moeten gequeried worden op een interval van lmqi. Dit heb ik dus ook geïmplementeerd.


---Gemaakt door Thibaut Van Goethem (S0171259)---
(Tot 1ste zit samen gemaakt met Miguel Dagrain)




