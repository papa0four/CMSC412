import java.io.BufferedReader; // reads text from char input stream
import java.io.FileNotFoundException; // handle no file error
import java.io.FileReader; // handle file read to stream
import java.io.IOException; // handle all IO operation fails
import java.util.Scanner; // allows for primitive input types
import java.util.StringTokenizer; // allows for token gen from strings

/* BankerAlgorithm class declaration */
public class BankerAlgorithm {

	/* everything will be done inside main */
	public static void main (String[] args) throws IOException, FileNotFoundException {

		/* local variable decalration for data storage */
		int numOfProcesses = 0;
		int numOfResources = 0;
		int lineCount = 0;
		int numOfCols[];
		int numOfRows[];
		int resourceVector[];
		int availableVector[];

		int currentAvailable[];
		int orderOfProcesses[];

		int index = 0;

		int maxResourceMatrix[][];
		int allocationMatrix[][];
		int needMatrix[][];

		boolean finish[];
		boolean isSafe = false;

		/* file IO variables used to store input text file data */
		String line;
		String filename;

		StringTokenizer lines;

		/* set variable input to scan and read text file data */
		Scanner input = new Scanner(System.in);

		/* prompt user to enter name of text file i.e. max.txt */
		System.out.print("Enter the name of the input file : ");
		filename = input.nextLine();

		/* build input stream and store in fr variable */
		FileReader fr = new FileReader(filename);

		/* use br variable to read the file one line at a time */
		BufferedReader br = new BufferedReader(fr);

		/* read the first line and store process data as int value */
		line = br.readLine();
		numOfProcesses = Integer.parseInt(line);

		/* read the next line and store resource data as int value */
		line = br.readLine();
		numOfResources = Integer.parseInt(line);

		/*
		 * create a multidimensional array to store each of
		 * the (m) processes and (n) resources.
		 */
		maxResourceMatrix = new int [numOfProcesses][numOfResources];
		allocationMatrix = new int [numOfProcesses][numOfResources];
		needMatrix = new int [numOfProcesses][numOfResources];

		/*
		 * initialize the following vectors and arrays:
		 * @resourceVector -> to store resources
		 * @availableVector -> to show what is still available
		 * @currentAvailable -> current resource being looked at
		 * @orderOfProcesses -> store prioritized processes
		 * @numOfCols -> store the number process and resource columns
		 * @numOfRows -> store the number of data within the text file by line
		 */
		resourceVector = new int [numOfResources];
		availableVector = new int [numOfResources];
		currentAvailable = new int [numOfResources];
		orderOfProcesses = new int [numOfProcesses];
		numOfCols = new int [numOfResources];
		numOfRows = new int [numOfProcesses];

		/* declare a bool array to determine if the process is finished executing */
		finish = new boolean[numOfProcesses];
		
		/* read the next line and set to line variable */
		line = br.readLine();

		/*
		 * loop through the file, reading the contained data
		 * and populating the maxResourceMatrix with the 
		 * appropriate data and converting the data into
		 * integer values.
		 */
		while (line != null && lineCount < numOfProcesses) {
			lines = new StringTokenizer(line);
			if (lines.hasMoreTokens()) {
				for (int i = 0; i < numOfResources; i++) {
					maxResourceMatrix[lineCount][i] = Integer.parseInt(lines.nextToken());
				}
			}

			line = br.readLine();
			lineCount++;
		}

		/* reset the local variable (lineCount) to 0 */
		lineCount = 0;

		/*
		 * re-read the data within the file and populate
		 * the allocationMatrix vector with the appropriate
		 * data and convert the values in to integer types.
		 */
		while (line != null && lineCount < numOfProcesses) {
			lines = new StringTokenizer(line);
			if (lines.hasMoreTokens()) {
				for (int i = 0; i < numOfResources; i++) {
					allocationMatrix[lineCount][i] = Integer.parseInt(lines.nextToken());
				}
			}

			line = br.readLine();
			lineCount++;
		}

		/* Read the last line of the text file and set the resource array */
		lines = new StringTokenizer(line);

		while (lines.hasMoreTokens()) {
			for (int i = 0; i < numOfResources; i++) {
				resourceVector[i] = Integer.parseInt(lines.nextToken());
			}
		}

		/* appropriately close the bufferedreader stream and text file */
		br.close();
		fr.close();

		/* 
		 * loop through the data within the maxResourcesMatrix
		 * and subtract the allocationMatrix from each column
		 * in order to determine the needMatrix values.
		 */
		for (int i = 0; i < numOfProcesses; i++) {
			for (int j = 0; j < numOfResources; j++) {
				needMatrix[i][j] = maxResourceMatrix[i][j] - allocationMatrix[i][j];
			}
		}

		/*
		 * loop through the number of processes and number
		 * of resources within the provided text file in order
		 * to determine and populate the available resource
		 * vector.
		 */
		for (int i = 0; i < numOfProcesses; i++) {
			for (int j = 0; j < numOfResources; j++) {
				numOfCols[j] += allocationMatrix[i][j];
				numOfRows[i] += needMatrix[i][j];
			}
		}

		for (int i = 0; i < numOfResources; i++) {
			availableVector[i] = resourceVector[i] - numOfCols[i];
		}

		/*
		 * iterate through the newly generated availableVector
		 * and look at the current index to properly determine
		 * the currently available resource. While iterating through
		 * let the process know that it is not finished and populate
		 * the finish array accordingly.
		 */
		for (int i = 0; i < numOfResources; i++) {
			currentAvailable[i] = availableVector[i];
		}

		for (int i = 0; i < numOfProcesses; i++) {
			finish[i] = false;
		}

		/* set a flag for unsafe state */
		boolean unsafe = false;

		/*
		 * utilize a do while loop in order to determine whether or
		 * not the system is currently in a safe state while
		 * executing the processes within their associated resources.
		 */
		do {
			/*
			 * process the state and determine whether or not the
			 * current state is in good standing given the resources
			 * available.
			 */
			unsafe = false;
			int i = 0;

			for (; i < numOfProcesses; i++) {

				/*
				 * if the current resource need is greater than
				 * the resources available, then that process will
				 * not be able to run and another process is evaluated.
				 */
				if ((!finish[i])) {
					boolean good = true;

					for (int j = 0; j < numOfResources; j++) {
						if (needMatrix[i][j] > currentAvailable[j]) {
							good = false;
							break;
						}
					}

					if (!good) {
						continue;
					}
					unsafe = true;
					break;
				}
			}

			/*
			 * if the current process is deemed eligible to run
			 * given the current available resources, then
			 * execute the current process.
			 */
			if (unsafe) {
				finish[i] = true;
				for (int j = 0; j < numOfResources; j++) {
					currentAvailable[j] += allocationMatrix[i][j];
				}

				/* increment the order of processes as they complete */
				orderOfProcesses[index++] = i;
			}
		} while (unsafe);

		/*
		 * iterate through all processes to determine
		 * whether or not they are in a finished state.
		 */
		for (int i = 0; i < numOfProcesses; i++) {
			if (!finish[i]) {
				isSafe = false;
			} else {
				isSafe = true;
			}
		}

		/*
		 * Display all of the matrices and vectors to show
		 * data interpreted from the input text file.
		 */
		System.out.println("Number of Processes : " + numOfProcesses);
		System.out.println("Number of Resources : " + numOfResources);

		System.out.println("Max Resource Matrix : ");
		for (int i = 0; i < numOfProcesses; i++) {
			for (int j = 0; j < numOfResources; j++) {
				System.out.print(maxResourceMatrix[i][j] + " ");
			}
			System.out.println();
		}

		System.out.println("\nAllocation Matrix : ");
		for (int i = 0; i < numOfProcesses; i++) {
			for (int j = 0; j < numOfResources; j++) {
				System.out.print(allocationMatrix[i][j] + " ");
			}
			System.out.println();
		}

		System.out.println("\nResource Vector : ");
		for (int i = 0; i < numOfResources; i++) {
			System.out.print(resourceVector[i] + " ");
		}
		System.out.println();

		System.out.println("\nNeed Matrix : ");
		for (int i = 0; i < numOfProcesses; i++) {
			for (int j = 0; j < numOfResources; j++) {
				System.out.print(needMatrix[i][j] + " ");
			}
			System.out.println();
		}

		System.out.println();
		System.out.println("Initial Available Vector : ");
		for (int i = 0; i < numOfResources; i++) {
			System.out.print(availableVector[i] + " ");
		}

		System.out.println("\n");

		/*
		 * print out the process sequence and whether or not
		 * the system executed within a safe state or not.
		 */
		System.out.println("Process Sequence of Execution:");
		if (isSafe) {
			for (int i = 0; i < orderOfProcesses.length; i++) {
				System.out.print((orderOfProcesses[i] + 1) + " ");
			}
			System.out.println();
			System.out.println("This system is in a safe state.");
		} else {
			System.out.println("This system is in an unsafe state.");
		}
	}
}