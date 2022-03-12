import java.io.FileNotFoundException;
import java.io.PrintWriter;

public class compute_a_posteriori {
    // The probability to get a CHERRY candy from a bag of type i = Hi_CHERRY
    // The probability to get a LIME candy from a bag of type i = 1 - Hi_CHERRY
    // i = 1, 2, 3, 4, 5
    static final double H1_CHERRY = 1;
    static final double H2_CHERRY = 0.75F;
    static final double H3_CHERRY = 0.5F;
    static final double H4_CHERRY = 0.25F;
    static final double H5_CHERRY = 0.0F;

    public static void main(String[] args) throws FileNotFoundException {
        char[] observations;
        int length = 0;
        PrintWriter writer = new PrintWriter("result.txt");

        double p_Q_equals_C = 0.5F;
        double p_Q_equals_L = 0.5F;

        double p_H1 = 0.1F;
        double p_H2 = 0.2F;
        double p_H3 = 0.4F;
        double p_H4 = 0.2F;
        double p_H5 = 0.1F;

        if (args.length < 1) {
            observations = new char[]{'n', 'o', 'n', 'e'};
        }
        else {
            observations = args[0].toCharArray();
            length = observations.length;
        }
        writer.println("Observation sequence Q: "+observations);
        writer.println("Length of Q: "+length);
        writer.println();

        for (int i = 0 ; i < length ; i++) {
            writer.println("After Observation "+(i+1)+" = "+observations[i]);
            writer.println();
            if (observations[i] == 'C') {
                p_H1 = (H1_CHERRY*p_H1)/p_Q_equals_C;
                p_H2 = (H2_CHERRY*p_H2)/p_Q_equals_C;
                p_H3 = (H3_CHERRY*p_H3)/p_Q_equals_C;
                p_H4 = (H4_CHERRY*p_H4)/p_Q_equals_C;
                p_H5 = (H5_CHERRY*p_H5)/p_Q_equals_C;
            }
            else if (observations[i] == 'L') {
                p_H1 = ((1-H1_CHERRY)*p_H1)/p_Q_equals_L;
                p_H2 = ((1-H2_CHERRY)*p_H2)/p_Q_equals_L;
                p_H3 = ((1-H3_CHERRY)*p_H3)/p_Q_equals_L;
                p_H4 = ((1-H4_CHERRY)*p_H4)/p_Q_equals_L;
                p_H5 = ((1-H5_CHERRY)*p_H5)/p_Q_equals_L;
            }
            else {
                System.out.println("Invalid observation. Skip.");
                continue;
            }
            p_Q_equals_C = (p_H1*H1_CHERRY)+(p_H2*H2_CHERRY)+(p_H3*H3_CHERRY)+(p_H4*H4_CHERRY)+(p_H5*H5_CHERRY);
            p_Q_equals_L = 1 - p_Q_equals_C;

            writer.format("P(h1 | Q) = %.5f\n", p_H1);
            writer.format("P(h2 | Q) = %.5f\n", p_H2);
            writer.format("P(h3 | Q) = %.5f\n", p_H3);
            writer.format("P(h4 | Q) = %.5f\n", p_H4);
            writer.format("P(h5 | Q) = %.5f\n\n", p_H5);

            writer.format("Probability that the next candy we pick will be C, given Q: %.5f\n", p_Q_equals_C);
            writer.format("Probability that the next candy we pick will be L, given Q: %.5f\n\n", p_Q_equals_L);
        }
        writer.close();
    }
}
