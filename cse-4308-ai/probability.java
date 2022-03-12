import java.io.*;
import java.util.*;

public class probability {
    // CountLine() used to count the number of lines in the data file,
    // or the number of records of training data in the file.
    public static int CountLine() throws IOException {
        int numLines = 0;
        BufferedReader reader = new BufferedReader(new FileReader("data.txt"));
        while (reader.readLine() != null) numLines++;
        if (reader != null) reader.close();
        return numLines;
    }
    // All training data will be stored in the 2D array called dataArr.
    // dataArr has 4 columns corresponding to data of the 4 nodes, and
    // its row is the number of records in file.
    public static int[][] ReadFileIntoArray(int numLines) throws FileNotFoundException {
        int [][] dataArr = new int[numLines][4];
        Scanner in = new Scanner(new BufferedReader(new FileReader("data.txt")));
        for (int i = 0 ; i < numLines && in.hasNextLine() ; i++) {
            // Each integer in the file is separated by 5-space distance.
            String [] line = in.nextLine().trim().split("     ");
            for (int j = 0 ; j < 4 ; j++) {
                dataArr[i][j] = Integer.parseInt(line[j]);
            }
        }
        return dataArr;
    }
    // baseball_game_on_TV is an independent node.
    public static void baseball_game_on_TV(int [][] dataArr, int numLines) {
        int game_played = 0;
        for (int i = 0 ; i < numLines ; i++) {
            if (dataArr[i][0] == 1) game_played++;
        }
        float prob = (float)game_played/(float)numLines;
        System.out.println("P(baseball_game_on_TV) = "+ prob);
        System.out.println("P(~baseball_game_on_TV) = "+ (1-prob));
    }
    // George_watches_TV has parent baseball_game_on_TV.
    public static void George_watches_TV(int [][] dataArr, int numLines) {
        // Given baseball_game_on_TV = TRUE ==> dataArr[i][0] = 1
        int watched = 0;
        int game_on_TV = 0;
        float prob = 0.0F;
        for (int i = 0 ; i < numLines ; i++) {
            if (dataArr[i][0] == 1) {
                game_on_TV++;
                if (dataArr[i][1] == 1) watched++;
            }
        }
        prob = (float)watched/(float)game_on_TV;
        System.out.println("P(George_watches_TV | baseball_game_on_TV) = "+prob);
        System.out.println("P(~George_watches_TV | baseball_game_on_TV) = "+(1-prob));

        // Given baseball_game_on_TV = FALSE ==> dataArr[i][0] = 0
        watched = 0;
        game_on_TV = 0;
        for (int i = 0 ; i < numLines ; i++) {
            if (dataArr[i][0] == 0) {
                game_on_TV++;
                if (dataArr[i][1] == 1) watched++;
            }
        }
        prob = (float)watched/(float)game_on_TV;
        System.out.println("P(George_watches_TV | ~baseball_game_on_TV) = "+prob);
        System.out.println("P(~George_watches_TV | ~baseball_game_on_TV) = "+(1-prob));
    }
    // out_of_cat_food is an independent node.
    public static void out_of_cat_food(int [][] dataArr, int numLines) {
        int out = 0;
        for (int i = 0 ; i < numLines ; i++) {
            if (dataArr[i][2] == 1) out++;
        }
        float prob = (float)out/(float)numLines;
        System.out.println("P(out_of_cat_food) = "+ prob);
        System.out.println("P(~out_of_cat_food) = "+ (1-prob));
    }
    // George_feeds_cat has 2 parents: baseball_game_on_TV an out_of_cat_food
    public static void George_feeds_cat(int [][] dataArr, int numLines) {
        float prob = 0.0F;
        // Given George_watches_TV = TRUE and out_of_cat_food = TRUE
        // ==> dataArr[i][1] = 1 and ==> dataArr[i][2] = 1
        int condition = 0;
        int fed = 0;
        for (int i = 0 ; i < numLines ; i++) {
            if (dataArr[i][1] == 1 && dataArr[i][2] == 1) {
                condition++;
                if (dataArr[i][3] == 1) fed++;
            }
        }
        prob = (float)fed/(float)condition;
        System.out.println("P(George_feeds_cat | George_watches_TV AND out_of_cat_food) = "+prob);
        System.out.println("P(~George_feeds_cat | George_watches_TV AND out_of_cat_food) = "+(1-prob));

        // Given George_watches_TV = TRUE and out_of_cat_food = FALSE
        // ==> dataArr[i][1] = 1 and ==> dataArr[i][2] = 0
        condition = 0;
        fed = 0;
        for (int i = 0 ; i < numLines ; i++) {
            if (dataArr[i][1] == 1 && dataArr[i][2] == 0) {
                condition++;
                if (dataArr[i][3] == 1) fed++;
            }
        }
        prob = (float)fed/(float)condition;
        System.out.println("P(George_feeds_cat | George_watches_TV AND ~out_of_cat_food) = "+prob);
        System.out.println("P(~George_feeds_cat | George_watches_TV AND ~out_of_cat_food) = "+(1-prob));

        // Given George_watches_TV = FALSE and out_of_cat_food = TRUE
        // ==> dataArr[i][1] = 0 and ==> dataArr[i][2] = 1
        condition = 0;
        fed = 0;
        for (int i = 0 ; i < numLines ; i++) {
            if (dataArr[i][1] == 0 && dataArr[i][2] == 1) {
                condition++;
                if (dataArr[i][3] == 1) fed++;
            }
        }
        prob = (float)fed/(float)condition;
        System.out.println("P(George_feeds_cat | ~George_watches_TV AND out_of_cat_food) = "+prob);
        System.out.println("P(~George_feeds_cat | ~George_watches_TV AND out_of_cat_food) = "+(1-prob));

        // Given George_watches_TV = FALSE and out_of_cat_food = FALSE
        // ==> dataArr[i][1] = 0 and ==> dataArr[i][2] = 0
        condition = 0;
        fed = 0;
        for (int i = 0 ; i < numLines ; i++) {
            if (dataArr[i][1] == 0 && dataArr[i][2] == 0) {
                condition++;
                if (dataArr[i][3] == 1) fed++;
            }
        }
        prob = (float)fed/(float)condition;
        System.out.println("P(George_feeds_cat | ~George_watches_TV AND ~out_of_cat_food) = "+prob);
        System.out.println("P(~George_feeds_cat | ~George_watches_TV AND ~out_of_cat_food) = "+(1-prob));
    }

    public static void main(String args[]) throws Exception {
        int numLines = CountLine();
        int [][] dataArr = ReadFileIntoArray(numLines);
        baseball_game_on_TV(dataArr, numLines);
        George_watches_TV(dataArr, numLines);
        out_of_cat_food(dataArr, numLines);
        George_feeds_cat(dataArr, numLines);
    }
}
