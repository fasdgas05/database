#include <iostream>
#include <stdio.h>
#include <Windows.h>
#include <sqlext.h>
#include <sql.h>
#include <iomanip>
#include <cstdlib>
#define STR_LEN 30
using namespace std;

HENV henv; // environment handle
HDBC hdbc; // connection handle
HSTMT hstmt; // statement handle
SQLRETURN retcode; // return code
UCHAR info[STR_LEN]; // info string for SQLGetInfo
SHORT cb_info_value;

SQLCHAR atrr_arr[10][30];
SQLCHAR outarr[10][30];//get string from query result
SQLLEN lOut[10];//get string length from query result

enum tableNames {
	movie, director, actor, customer, award, genre, movieGenre, movieObtain, actorObatin,
	directorObtain, casting, make, customerRate
}; // table names to check
bool created[13] = {}; // check tables is created or not

void createTables();
void init();
void delTables();

void execQuery(SQLCHAR* qr) {
	retcode = SQLAllocStmt(hdbc, &hstmt);
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		retcode = SQLExecDirect(hstmt, qr, SQL_NTS);
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			cout << "Translated SQL : " << qr << "\n\n";
			return;
		}
		else {
			cout << "Error with qeury : \n" << qr << "\n";
			delTables();
			abort();
		}
	}
	else {
		cout << "Allocate error\n";
		delTables();
		abort();
	}
}

void execQuerySelect(SQLCHAR* qr) {
	retcode = SQLAllocStmt(hdbc, &hstmt);
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		retcode = SQLExecDirect(hstmt, qr, SQL_NTS);
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			cout << "Translated SQL : " << qr << "\n\n";
			SQLSMALLINT count;
			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
				SQLCHAR atrr_arr[10][30] = {};
				SQLCHAR outarr[10][30] = {};//get string from query result
				SQLLEN lOut[10] = {};//get string length from query result
				retcode = SQLNumResultCols(hstmt, &count); // get column count
				SQLLEN row_count;
				SQLRowCount(hstmt, &row_count);// get row count
				if (row_count == 0) { // if there is no row
					cout << "null table\n\n";
					return;
				}
				for (int i = 1; i <= count; i++) {
					retcode = SQLColAttributes(hstmt, i, SQL_CHAR, atrr_arr[i], sizeof(atrr_arr[i]), NULL, &lOut[i]); // get column names
					printf("%-30s", atrr_arr[i]);
				}
				printf("\n\n");
				for (int i = 1; i <= count; i++) {
					SQLBindCol(hstmt, i, SQL_CHAR, outarr[i], sizeof(outarr[i]), &lOut[i]);
				}
				while (SQLFetch(hstmt) != SQL_NO_DATA) {
					for (int i = 1; i <= count; i++) {
						printf("%-30s", outarr[i]);
					}
					printf("\n");
				}
			}
			printf("\n");
			return;
		}
		else {
			cout << "Error with qeury : \n" << qr << "\n";
			delTables();
			abort();
		}
	}
	else {
		cout << "Allocate error\n";
		delTables();
		abort();
	}
}

void createTables() {
	cout << "Create tables\n\n";
	execQuery((SQLCHAR*)"create table movie (movieID integer primary key, movieName varchar(30)"
		", releaseYear char(5), releaseMonth char(3), releaseDate char(3), publisherName varchar(30), avgRate numeric(2,1))");
	created[movie] = true;
	execQuery((SQLCHAR*)"create table director(directorID integer primary key, directorName varchar(20),"
		"dateOfBirth char(12),dateOfDeath char(12))");
	created[director] = true;
	execQuery((SQLCHAR*)"create table actor(actorID integer primary key, actorName varchar(20),"
		"dateOfBirth char(12), dateOfDeath char(12), gender varchar(6))");
	created[actor] = true;
	execQuery((SQLCHAR*)"create table customer(customerID integer primary key, customerName varchar(20),"
		"dateOfBirth char(12), gender char(6))");
	created[customer] = true;
	execQuery((SQLCHAR*)"create table award(awardID integer, awardName varchar(30),primary key(awardID))");
	created[award] = true;
	execQuery((SQLCHAR*)"create table genre(genreName varchar(15) primary key)");
	created[genre] = true;
	execQuery((SQLCHAR*)"create table movieGenre(movieID integer, genreName varchar(15),"
		"foreign key(movieID) references movie(movieID) on delete cascade,foreign key(genreName) references genre(genreName) on delete cascade"
		", primary key(movieID, genreName))");
	created[movieGenre] = true;
	execQuery((SQLCHAR*)"create table movieObtain(movieID integer, awardID integer,"
		"year char(4), primary key(movieID, awardID),"
		"foreign key(movieID) references movie(movieID) on delete cascade, foreign key(awardID) references award(awardID) on delete cascade)");
	created[movieObtain] = true;
	execQuery((SQLCHAR*)"create table actorObtain(actorID integer, awardID integer,"
		"year char(4),primary key(actorID,awardID), foreign key(actorID) references actor(actorID) on delete cascade,"
		" foreign key(awardID)  references award(awardID) on delete cascade)");
	created[actorObatin] = true;
	execQuery((SQLCHAR*)"create table directorObtain(directorID integer, awardID integer,"
		"year char(4),primary key(directorID, awardID) , foreign key(directorID) references director(directorID) on delete cascade,"
		" foreign key(awardID) references award(awardID) on delete cascade)");
	created[directorObtain] = true;
	execQuery((SQLCHAR*)"create table casting(movieID integer, actorID integer, role varchar(30),"
		"primary key(movieID, actorID),foreign key(movieID) references movie(movieID) on delete cascade,"
		"foreign key(actorID) references actor(actorID) on delete cascade)");
	created[casting] = true;
	execQuery((SQLCHAR*)"create table make(movieID integer,directorID integer,primary key(movieID, directorID),"
		" foreign key(movieID) references movie(movieID) on delete cascade,"
		"foreign key(directorID) references director(directorID) on delete cascade);");
	created[make] = true;
	execQuery((SQLCHAR*)"create table customerRate(customerID integer, movieID integer, rate numeric(2,1),"
		"primary key(customerID, movieID),"
		"foreign key(customerID) references customer(customerID) on delete cascade, foreign key(movieID) references movie(movieID) on delete cascade)");
	created[customerRate] = true;
	cout << "\ntable created\n";

}

void init() {
	//initialize tables
	cout << "\nInitialize talbes\n\n";
	//director
	execQuery((SQLCHAR*)"insert into director "
		"values(1, 'Tim Burton', '1958.8.25', null)");
	execQuery((SQLCHAR*)"insert into director "
		"values(2, 'David Fincher', '1962.8.28', null)");
	execQuery((SQLCHAR*)"insert into director "
		"values(3, 'Christopher Nolan', '1970.7.30', null)");

	//actor
	execQuery((SQLCHAR*)"insert into actor "
		"values(1,'Johnny Depp','1963.6.9',null,'Male')");
	execQuery((SQLCHAR*)"insert into actor "
		"values(2, 'Winona Ryder','1971.10.29',null,'Female')");
	execQuery((SQLCHAR*)"insert into actor "
		"values(3,'Anne Hathaway','1982.11.12',null,'Female')");
	execQuery((SQLCHAR*)"insert into actor "
		"values(4, 'Christian Bale','1974.1.30',null,'Male')");
	execQuery((SQLCHAR*)"insert into actor "
		"values(5,'Heath Ledger','1794.4.4','2008.1.22','Male')");
	execQuery((SQLCHAR*)"insert into actor "
		"values(6,'Jesse Eisenberg','1983.10.5',null,'Male')");
	execQuery((SQLCHAR*)"insert into actor "
		"values(7,'Andrew Garfield','1983.8.20',null,'Male')");

	//customer
	execQuery((SQLCHAR*)"insert into customer "
		"values(1,'Bob','1997.11.14','Male')");
	execQuery((SQLCHAR*)"insert into customer "
		"values(2,'John','1978.01.23','Male')");
	execQuery((SQLCHAR*)"insert into customer "
		"values(3,'Jack','1980.05.04','Male')");
	execQuery((SQLCHAR*)"insert into customer "
		"values(4,'Jill','1981.04.17','Female')");
	execQuery((SQLCHAR*)"insert into customer "
		"values(5,'Bell','1990.05.14','Female')");

	//movie
	execQuery((SQLCHAR*)"insert into movie "
		"values(1,'Edward Scissorhands','1991','06',"
		"'29','20th Century Fox Presents',0.0)");
	execQuery((SQLCHAR*)"insert into movie "
		"values(2, 'Alice In Wonderland','2010','03','04',"
		"'Korea Sonry Pictures',0.0)");
	execQuery((SQLCHAR*)"insert into movie "
		"values(3,'The Social Network','2010','11','18',"
		"'Korea Sony Pictures',0.0)");
	execQuery((SQLCHAR*)"insert into movie "
		"values(4, 'The Dark Knight','2008','08','06',"
		"'Warner Brothers Korea',0.0)");

	//genre
	execQuery((SQLCHAR*)"insert into genre "
		"values('Fantasy')");
	execQuery((SQLCHAR*)"insert into genre "
		"values('Romance')");
	execQuery((SQLCHAR*)"insert into genre "
		"values('Adventure')");
	execQuery((SQLCHAR*)"insert into genre "
		"values('Family')");
	execQuery((SQLCHAR*)"insert into genre "
		"values('Drama')");
	execQuery((SQLCHAR*)"insert into genre "
		"values('Action')");
	execQuery((SQLCHAR*)"insert into genre "
		"values('Mystery')");
	execQuery((SQLCHAR*)"insert into genre "
		"values('Thriller')");

	//make
	execQuery((SQLCHAR*)"insert into make "
		"values(1,1)");
	execQuery((SQLCHAR*)"insert into make "
		"values(2,1)");
	execQuery((SQLCHAR*)"insert into make "
		"values(3,2)");
	execQuery((SQLCHAR*)"insert into make "
		"values(4,3)");

	//casting
	execQuery((SQLCHAR*)"insert into casting "
		"values(1,1,'Main actor')");
	execQuery((SQLCHAR*)"insert into casting "
		"values(1,2,'Main actor')");
	execQuery((SQLCHAR*)"insert into casting "
		"values(2,2,'Main actor')");
	execQuery((SQLCHAR*)"insert into casting "
		"values(2,3,'Main actor')");
	execQuery((SQLCHAR*)"insert into casting "
		"values(3,6,'Main actor')");
	execQuery((SQLCHAR*)"insert into casting "
		"values(3,7,'Supporting Actor')");
	execQuery((SQLCHAR*)"insert into casting "
		"values(4,4,'Main actor')");
	execQuery((SQLCHAR*)"insert into casting "
		"values(4,5,'Main actor')");

	//movieGenre
	execQuery((SQLCHAR*)"insert into movieGenre "
		"values(1,'Fantasy'),"
		"(1,'Romance')");
	execQuery((SQLCHAR*)"insert into movieGenre "
		"values(2,'Fantasy'),"
		"(2,'Adventure'),"
		"(2,'Family')");
	execQuery((SQLCHAR*)"insert into movieGenre "
		"values(3,'Drama')");
	execQuery((SQLCHAR*)"insert into movieGenre "
		"values(4,'Action'),"
		"(4,'Drama'),"
		"(4,'Mystery'),"
		"(4,'Thriller')");
	cout << "\ninitial data inserted\n\n";
}

void delTables() {
	cout << "\nDrop all tables\n";
	if (created[customerRate])
		execQuery((SQLCHAR*)"drop table customerRate");
	if (created[make])
		execQuery((SQLCHAR*)"drop table make");
	if (created[casting])
		execQuery((SQLCHAR*)"drop table casting");
	if (created[directorObtain])
		execQuery((SQLCHAR*)"drop table directorObtain");
	if (created[actorObatin])
		execQuery((SQLCHAR*)"drop table actorObtain");
	if (created[movieObtain])
		execQuery((SQLCHAR*)"drop table movieObtain");
	if (created[movieGenre])
		execQuery((SQLCHAR*)"drop table movieGenre");
	if (created[movie])
		execQuery((SQLCHAR*)"drop table movie");
	if (created[director])
		execQuery((SQLCHAR*)"drop table director");
	if (created[actor])
		execQuery((SQLCHAR*)"drop table actor");
	if (created[customer])
		execQuery((SQLCHAR*)"drop table customer");
	if (created[award])
		execQuery((SQLCHAR*)"drop table award");
	if (created[genre])
		execQuery((SQLCHAR*)"drop table genre");
	cout << "\ntable dropped\n";
}

int main() {
	SQLAllocEnv(&henv);
	SQLAllocConnect(henv, &hdbc);
	SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
	retcode = SQLConnect(hdbc, (SQLCHAR*)"mysql_odbc", SQL_NTS, (SQLCHAR*)"root", SQL_NTS, (SQLCHAR*)"cs3207", SQL_NTS);
	if (retcode == SQL_SUCCESS) {
		retcode = SQLGetInfo(hdbc, SQL_DBMS_VER, &info, STR_LEN, &cb_info_value);
		cout << "connection sucessful" << endl;
		cout << "current DBMS version is " << info << endl;

		//1.
		createTables();
		init();

		//2. Insert the proper data from the following statements.
		cout << "2. Insert the proper data from the following statements.\n\n";

		//2.1. Winona Ryder won the ¡°Best supporting actor¡± 
		//award in 1994
		cout << "Statement : Winona Ryder won the ¡°Best supporting actor¡± award in 1994\n";
		execQuery((SQLCHAR*)"insert into award "
			"values(1,'Best supporting actor')");
		execQuery((SQLCHAR*)"insert into actorObtain "
			"values(2, 1,'1994')");
		cout << "award table\n";
		execQuerySelect((SQLCHAR*)"select * from award");
		cout << "actorObatin table\n";
		execQuerySelect((SQLCHAR*)"select * from actorObtain");


		//2.2. Andrew Garfield won the ¡°Best supporting actor¡± 
		//award in 2011
		cout << "Statement : Andrew Garfield won the ¡°Best supporting actor¡± award in 2011\n";
		execQuery((SQLCHAR*)"insert into actorObtain "
			"values(7,1,'2011')");
		cout << "actorObatin table\n";
		execQuerySelect((SQLCHAR*)"select * from actorObtain");


		//2.3. Jesse Eisenberg won the ¡°Best main actor¡± 
		//award in 2011
		cout << "Statement : Jesse Eisenberg won the ¡°Best main actor¡± award in 2011\n";
		execQuery((SQLCHAR*)"insert into award "
			"values(2,'Best main actor')");
		execQuery((SQLCHAR*)"insert into actorObtain "
			"values(6,2,'2011')");
		cout << "Updated tables\n";
		cout << "award table\n";
		execQuerySelect((SQLCHAR*)"select * from award");
		cout << "actorObatin table\n";
		execQuerySelect((SQLCHAR*)"select * from actorObtain");


		//2.4. Johnny Depp won the ¡°Best villain actor¡± 
		//award in 2011
		cout << "Statement : Johnny Depp won the ¡°Best villain actor¡± award in 2011\n";
		execQuery((SQLCHAR*)"insert into award "
			"values(3, 'Best villain actor')");
		execQuery((SQLCHAR*)"insert into actorObtain "
			"values(1,3,'2011')");
		cout << "Updated tables\n";
		cout << "award table\n";
		execQuerySelect((SQLCHAR*)"select * from award");
		cout << "actorObatin table\n";
		execQuerySelect((SQLCHAR*)"select * from actorObtain");


		//2.5. Edward Scissorhands won the 
		//¡°Best fantasy movie¡± award in 1991
		cout << "Statement : Edward Scissorhands won the ¡°Best fantasy movie¡± award in 1991\n";
		execQuery((SQLCHAR*)"insert into award "
			"values(4,'Best fantasy movie')");
		execQuery((SQLCHAR*)"insert into movieObtain "
			"values(1,4,'1991')");
		cout << "Updated tables\n";
		cout << "award table\n";
		execQuerySelect((SQLCHAR*)"select * from award");
		cout << "movieObtain table\n";
		execQuerySelect((SQLCHAR*)"select * from movieObtain");


		//2.6. Alice In Wonderland won the 
		//¡°Best fantasy movie¡± award in 2011
		cout << "Statement : Alice In Wonderland won the ¡°Best fantasy movie¡± award in 2011\n";
		execQuery((SQLCHAR*)"insert into movieObtain "
			"values(2,4,'2011')");
		cout << "Updated tables\n";
		cout << "movieObtain table\n";
		execQuerySelect((SQLCHAR*)"select * from movieObtain");


		//3. Insert data to the proper tables based on the following statements and update avgRate if necessary.
		cout << "3. Insert data to the proper tables based on the following statements and update avgRate if necessary.\n\n";
		//3.1 Bob rates 5 to ¡°The Dark Knight¡±.
		cout << "Statement : Bob rates 5 to ¡°The Dark Knight¡±.\n";
		execQuery((SQLCHAR*)"insert into customerRate "
			"values(1,4,5.0)");
		execQuery((SQLCHAR*)"update movie "
			"set avgRate = (select avg(rate) from customerRate where movieID=4) "
			"where movieID=4");
		cout << "Updated tables\n";
		cout << "customerRate table\n";
		execQuerySelect((SQLCHAR*)"select * from customerRate");
		cout << "movie table\n";
		execQuerySelect((SQLCHAR*)"select * from movie");


		//3.2 Bell rates 5 to the movies whose director is ¡°Tim Burton¡±
		cout << "Statement : Bell rates 5 to the movies whose director is ¡°Tim Burton¡±\n";
		execQuery((SQLCHAR*)"insert into customerRate "
			"select 5 as customerID, movieID, 5.0 as rate from make where directorID = 1");
		execQuery((SQLCHAR*)"update movie m "
			"set avgRate = (select avg(rate) "
			"from (select movieID, rate "
			"from make natural join customerRate "
			"where m.movieID=movieID) s)"
			"where m.movieID in (select movieID from make where directorID=1)");
		cout << "Updated tables\n";
		cout << "customerRate table\n";
		execQuerySelect((SQLCHAR*)"select * from customerRate");
		cout << "movie table\n";
		execQuerySelect((SQLCHAR*)"select * from movie");

		//3.3 Jill rates 4 to the movies whose main actor is female
		cout << "Statement : Jill rates 4 to the movies whose main actor is female\n";
		execQuery((SQLCHAR*)"insert into customerRate "
			"select distinct 4 as customerID, movieID, 4.0 as rate from casting natural join actor "
			"where gender = 'Female'");
		execQuery((SQLCHAR*)"update movie m "
			"set avgRate = (select avg(rate) from (select movieID, rate "
			"from actor natural join casting natural join customerRate "
			"where m.movieID=movieID)s)"
			"where m.movieID in (select movieID from casting natural join actor where gender='Female')");
		cout << "Updated tables\n";
		cout << "customerRate table\n";
		execQuerySelect((SQLCHAR*)"select * from customerRate");
		cout << "movie table\n";
		execQuerySelect((SQLCHAR*)"select * from movie");

		//3.4 Jack rates 4 to the fantasy movies.
		cout << "Statement : Jack rates 4 to the fantasy movies.\n";
		execQuery((SQLCHAR*)"insert into customerRate "
			"select distinct 3 as customerID, movieID, 4.0 as rate from movie natural join movieGenre "
			"where genreName='Fantasy'");
		execQuery((SQLCHAR*)"update movie m "
			"set avgRate = (select avg(rate) from (select movieID, rate "
			"from movieGenre natural join customerRate "
			"where m.movieID = movieID)s)"
			"where m.movieID in (select movieID from movieGenre where genreName='Fantasy')");
		cout << "Updated tables\n";
		cout << "customerRate table\n";
		execQuerySelect((SQLCHAR*)"select * from customerRate");
		cout << "movie table\n";
		execQuerySelect((SQLCHAR*)"select * from movie");


		//4. Select the names of the movies whose actor are dead.
		cout << "Statement : Select the names of the movies whose actor are dead.\n\n";
		execQuerySelect((SQLCHAR*)"select distinct movieName "
			"from movie natural join casting natural join actor "
			"where dateOfDeath is not null");

		//5. Select the names of the directors 
		//who cast the same actor more than once.
		cout << "Statement : Select the names of the directors who cast the same actor more than once.\n\n";
		execQuerySelect((SQLCHAR*)"select directorName "
			"from casting natural join make natural join director "
			"group by actorID, directorID having count(*) > 1");

		//6. Select the names of the movies and the genres, 
		//where movies have the common genre.
		cout << "Statement : Select the names of the movies and the genres, where movies have the common genre.\n\n";
		execQuerySelect((SQLCHAR*)"select movieName, genreName "
			"from movie natural join movieGenre "
			"where genreName in (select genreName "
			"from movieGenre "
			"group by genreName "
			"having count(genreName) > 1)");

		//7. Delete the movies whose director or actor did not get any award
		//and delete data from related tables.
		cout << "Statement : Delete the movies whose director or actor did not get any award and delete data from related tables.\n";
		execQuery((SQLCHAR*)"delete from movie where movie.movieID not in (select movieID "
			"from casting natural join actorObtain) and movie.movieID not in (select movieID "
			"from make natural join directorObtain)");
		cout << "Updated tables\n";
		cout << "movie table\n";
		execQuerySelect((SQLCHAR*)"select * from movie");
		cout << "casting table\n";
		execQuerySelect((SQLCHAR*)"select * from casting");
		cout << "make table\n";
		execQuerySelect((SQLCHAR*)"select * from make");
		cout << "movieGenre table\n";
		execQuerySelect((SQLCHAR*)"select * from movieGenre");
		cout << "movieObtain table\n";
		execQuerySelect((SQLCHAR*)"select * from movieObtain");
		cout << "customerRate table\n";
		execQuerySelect((SQLCHAR*)"select * from customerRate");

		//8. Delete all customers and delete data from related tables.
		cout << "Statement : Delete all customers and delete data from related tables.\n";
		execQuery((SQLCHAR*)"delete from customer");
		execQuery((SQLCHAR*)"update movie "
			"set avgRate = (select avg(rate) from customerRate "
			"group by movieID having customerRate.movieID=movie.movieID)");
		cout << "Updated tables\n";
		cout << "customer table\n";
		execQuerySelect((SQLCHAR*)"select * from customer");
		cout << "customerRate table\n";
		execQuerySelect((SQLCHAR*)"select * from customerRate");
		cout << "movie table\n";
		execQuerySelect((SQLCHAR*)"select * from movie");

		delTables();

		if (hstmt)
			SQLCloseCursor(hstmt);
		SQLDisconnect(hdbc);
		SQLFreeConnect(hdbc);
		SQLFreeEnv(henv);
	}
	else {
		cout << "connection failed" << endl;
		SQLDisconnect(hdbc);
		SQLFreeConnect(hdbc);
		SQLFreeEnv(henv);
	}
}