Excel and every other spreadsheet program that I know of is terrible at reading CSV files.  If you use CSV files regularly, you probably know what I mean.  It tries to interpret the information as data types, and it doesn't do a very good job of determining what the data is.  One example is a CSV file that I had that contained the value "9470628211241443".  That was a reference value and I needed to be able to get that exact value, but LibreOffice interpreted it as a floating point number and displayed it as "9.47062821124144E+015".

But that's not really the fault of Excel or any other spreadsheet software.  It's just not designed for that.  Spreadsheets are for making calcuations, but CSVs are for storing data.  We use Excel to open a CSV because it kinda/sorta looks like a spreadsheet, but it's not a spreadsheet.

Constantly frustrated with having no good solution to viewing data in a CSV file, I finally decided to make this program to read CSV files without the problem of spreadsheets showing me misinterpreted data.

csv.c (and its header file) is taken from here-- https://github.com/semitrivial/csv_parser/tree/master.  It's been modded by adding a delimiter option and making one static function non-static.  I am immensely grateful to its author for making this project much easier.

I'm really just making this for myself, so I'm only adding features that I need at the moment.  It only reads and does not edit, for starters.  More functions may be added as-needed in the future, but if you're reading this and you need a new feature, you'll probably need to fork the project and add it yourself.

## Example usage (for Linux):

Normal view:

`csview < /path/to/csv/file`

       +-----------------------------------------------------------------------------------------------+
       |Last Name      |First Name     |Cell Number    |VIN            |Customer ID    |Purchase Amount|
       +-----------------------------------------------------------------------------------------------+
      1|Doe            |John           |123-456-7890   |1HGCM82633A0012|123456789012345|100.50         |
      2|Smith          |Jane           |987-654-3210   |2LMHJ5FR9GBL123|234567890123456|250.75         |
      3|Johnson        |Michael        |555-555-5555   |5YJ3E1EA7JF1234|345678901234567|300.25         |
      4|Williams       |Emily          |333-333-3333   |1FTEW1EP1LKD123|456789012345678|150.00         |
       +-----------------------------------------------------------------------------------------------+

Transposed output:

`csview -o t < /path/to/csv/file`

                    1               2               3               4
    -------------------------------------------------------------------------------+
    [Last Name]    |Doe            |Smith          |Johnson        |Williams       |
    [First Name]   |John           |Jane           |Michael        |Emily          |
    [Cell Number]  |123-456-7890   |987-654-3210   |555-555-5555   |333-333-3333   |
    [VIN]          |1HGCM82633A0012|2LMHJ5FR9GBL123|5YJ3E1EA7JF1234|1FTEW1EP1LKD123|
    [Customer ID]  |123456789012345|234567890123456|345678901234567|456789012345678|
    [Purchase Amou]|100.50         |250.75         |300.25         |150.00         |
    -------------------------------------------------------------------------------+

Vertical output:

`csview -o v < /path/to/csv/file`

    *************** Line   1 ***************
    Last Name: Doe
    First Name: John
    Cell Number: 123-456-7890
    VIN: 1HGCM82633A001234
    Customer ID: 123456789012345
    Purchase Amount: 100.50
    *************** Line   2 ***************
    Last Name: Smith
    First Name: Jane
    Cell Number: 987-654-3210
    VIN: 2LMHJ5FR9GBL12345
    Customer ID: 234567890123456
    Purchase Amount: 250.75
    *************** Line   3 ***************
    Last Name: Johnson
    First Name: Michael
    Cell Number: 555-555-5555
    VIN: 5YJ3E1EA7JF123456
    Customer ID: 345678901234567
    Purchase Amount: 300.25
    *************** Line   4 ***************
    Last Name: Williams
    First Name: Emily
    Cell Number: 333-333-3333
    VIN: 1FTEW1EP1LKD12345
    Customer ID: 456789012345678
    Purchase Amount: 150.00

Raw output:

`csview -o r < /path/to/csv/file`

    Last Name,First Name,Cell Number,VIN,Customer ID,Purchase Amount
    Doe,John,123-456-7890,1HGCM82633A001234,123456789012345,100.50
    Smith,Jane,987-654-3210,2LMHJ5FR9GBL12345,234567890123456,250.75
    Johnson,Michael,555-555-5555,5YJ3E1EA7JF123456,345678901234567,300.25
    Williams,Emily,333-333-3333,1FTEW1EP1LKD12345,456789012345678,150.00

Other options by example (they're weird, I know):

`csview -h < /path/to/csv/file` (Headers) Prints just the headers.

`csview -w 20 < /path/to/csv/file` (Width) Changes width to 20.

`csview -n < /path/to/csv/file` (No header) Reads the file as if it has no headers

`csview -d '|' < /path/to/csv/file` (Delimiter) Changes the delimiter to |

`csview -k 2 < /path/to/csv/file` (sKip) Skips the first 2 lines.

`csview -f "Last Name,Customer ID" < /path/to/csv/file` (Field) Shows just Last Name and Customer ID columns. (Note: If you get a "Segmentation Fault" error, that probably means you mistyped a field name!  I'll try to fix that sometime.)

`csview -r l "2-5,7,10-14" < /path/to/csv/file` (Restrict by Lines) Only displays lines in those ranges.

`csview -r r "Purchase Amount" "50-175,300-700" < /path/to/csv/file` (Restrict by Range) Only display lines where the value in Purchase Amount column falls in one of the given ranges.

`csview -r e "First Name" "John,Jane" < /path/to/csv/file` (Restrict by Equals) Only display lines where value in First Name column equals John or Jane.

`csview -s < /path/to/csv/file` (Suppress line numbers) Don't show line numbers.  Works in normal, transposed, and vertical output, but does nothing for raw output (which doesn't show line numbers anyway).
