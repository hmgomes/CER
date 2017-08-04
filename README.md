# Cause Effect Rules algorithm
Repository for the Cause Effect Rules (CER). 

**IMPORTANT:** This code was implemented a few years ago and I haven't updated it since. Also, it might output messages in portuguese, which may be complicate for non-portuguese speakers to handle. If you have any issues while executing this code, please contact the main author (www.heitorgomes.com, look for Contact tab). 

## Citing and obtaining more information about Cause Effect Rules
To cite CER in a publication, please cite the following paper: 
> Heitor Murilo Gomes, Deborah Ribeiro Carvalho, Lourdes Zubieta, Jean Paul Barddal, and Andreia Malucelli. 2015. On the Discovery of Time Distance Constrained Temporal Association Rules. In Proceeings, Part II, of the 22nd International Conference on Neural Information Processing - Volume 9490 (ICONIP 2015), Vol. 9490. Springer-Verlag New York, Inc., New York, NY, USA, 510-519. DOI: http://dx.doi.org/10.1007/978-3-319-26535-3_58

## How to execute it
The algorithms runs on the command line and expects some parameters to define input files as well as other thresholds. 
The most important parameters are:
* Minimum Event Frequency (-FME): The minimum threshold for the event frequency. 
* Minimum Support CE (-SMCE): The minimum threshold for support. 
* Maximum Cause set size (-MC): The maximum number of events in the cause set of rules. 
* Minimum Confidence CE (-CMCE): The minimum threshold for confidence. 
* Time Distance Restriction file: This file contains the time distance restrictions, i.e., rules will be mined based on the minimum and maximum time distances defined on this file using the format _year:month:day:hour:minute:second_ to define minimum and maximum (separated by _;_). Sample:

_0000:00:00:00:00:01;0000:00:01:00:00:00_
_0000:00:00:01:00:00;0000:00:04:00:00:00_
_0000:00:01:00:00:00;0000:00:07:00:00:00_

This file would define 3 TDRs: 1 second ~ 1 day, 1 hour ~ 4 days and 1 day ~ 7 days. 

* Input events file: This file contains the events to be mined. Each row should be associated with the same entity (e.g. one patient) and each event must contain a timestamp. Sample: 

_Acerola(10/5/2010-15:54:31) Abobora(10/5/2010-15:57:44) Abobora(10/5/2010-16:00:00)_ 
_Abobora(5/18/2010-11:44:42) Acerola(5/18/2010-11:45:30) Abacaxi(5/18/2010-11:46:30) Ameixa(5/18/2010-11:47:00)_ 
_Ameixa(3/22/2010-14:9:42) Acerola(3/22/2010-14:10:00) Acerola(3/22/2010-14:10:30) Acerola(3/22/2010-14:11:05)_ 
_Acerola(3/15/2010-6:17:48) Abacaxi(3/15/2010-6:18:10) Abacaxi(3/15/2010-6:18:20) Acelga(3/15/2010-6:18:40)_ 
_Abobora(2/9/2010-3:4:51) Abacaxi(2/9/2010-3:5:40) Abacaxi(2/9/2010-3:6:00)_

**Sample command (unix):** 
`./CER1 -dsample.txt -rsample_tdr.txt`

**Explanation:** Mine CE rules from sample.txt using sample_tdr to define the time distances (you can find these sample files in this repository). Since no output file was defined, it should output a sqlite database named after the current time sample, for example: _ 2017-8-4_10h39m54s.sqlite_. 

**Inspecting the output:** To view the output it is necessary to inspect the generated SQLite database. All the results should be available in an easy to understand format through view named "resultado". You can also create your own ways to manipulate the output by applying SELECTs to the RCE table. Unfortunately, these tables and meta-data in general are in portuguese (e.g. RCE instead of CER), this may or may not be changed in the future. 

## Dependencies:
Make sure to include libsqlite3 to the project. 
