package lab5;

import java.util.Random;
import java.util.Scanner;

public class lab5 {
    private static final Random random = new Random();
    private static final Scanner scanner = new Scanner(System.in);

    public static void main(String[] args) {
        System.out.println("Welcome to the Coin Game!");
        System.out.println("Rules: Players take turns taking 1 or 2 coins from the pile.");
        System.out.println("The player who takes the last coin wins.");

        // Generate random number of coins (between 10 and 30)
        int totalCoins = random.nextInt(21) + 10;
        System.out.println("Starting with " + totalCoins + " coins.");

        // Determine who goes first randomly
        boolean isUserTurn = random.nextBoolean();
        if (isUserTurn) {
            System.out.println("You go first!");
        } else {
            System.out.println("Computer goes first!");
        }

        // Game loop
        while (totalCoins > 0) {
            displayCoins(totalCoins);
            
            if (isUserTurn) {
                totalCoins = userMove(totalCoins);
            } else {
                totalCoins = computerMove(totalCoins);
            }
            
            // Switch turns
            isUserTurn = !isUserTurn;
        }

        // Game over
        if (isUserTurn) {
            System.out.println("Computer took the last coin. Computer wins!");
        } else {
            System.out.println("You took the last coin. You win!");
        }
        
        scanner.close();
    }

    private static void displayCoins(int coins) {
        System.out.println("\nCoins remaining: " + coins);
        for (int i = 0; i < coins; i++) {
            System.out.print("● ");
        }
        System.out.println();
    }

    private static int userMove(int coins) {
        int take;
        while (true) {
            System.out.print("How many coins would you like to take? (1 or 2): ");
            try {
                take = Integer.parseInt(scanner.nextLine());
                if (take == 1 || take == 2) {
                    if (take <= coins) {
                        break;
                    } else {
                        System.out.println("There aren't that many coins left! Try again.");
                    }
                } else {
                    System.out.println("You must take either 1 or 2 coins. Try again.");
                }
            } catch (NumberFormatException e) {
                System.out.println("Please enter a valid number (1 or 2).");
            }
        }
        
        System.out.println("You took " + take + " coin(s).");
        return coins - take;
    }

    private static int computerMove(int coins) {
        // Optimal strategy: always try to leave a multiple of 3 coins for the opponent
        int take;
        
        if (coins == 1) {
            // If only one coin left, have to take it
            take = 1;
        } else if (coins == 2) {
            // If two coins left, take both to win
            take = 2;
        } else if (coins % 3 == 0) {
            // If coins are multiple of 3, can't force a win
            // Just take 1 and hope for opponent mistake
            take = 1;
        } else if (coins % 3 == 1) {
            // Take 1 to leave multiple of 3
            take = 1;
        } else { // coins % 3 == 2
            // Take 2 to leave multiple of 3
            take = 2;
        }
        
        System.out.println("Computer takes " + take + " coin(s).");
        return coins - take;
    }
}