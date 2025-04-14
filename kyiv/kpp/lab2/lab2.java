package lab2;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JTextField;
import javax.swing.BoxLayout;
import java.awt.Component;

class lab2 extends JFrame{
    public static void main(String[] args) {
        JFrame f = new JFrame("converter");

        f.setVisible(true);
        f.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);

        f.setSize(500, 500);
        f.setLocation(430, 100);

        JPanel panel = new JPanel();
        panel.setLayout(new BoxLayout(panel, BoxLayout.Y_AXIS));

        f.add(panel);

        JLabel lbl = new JLabel("Select one of the possible choices and click OK");
        lbl.setAlignmentX(Component.CENTER_ALIGNMENT);

        panel.add(lbl);







        JTextField inputField = new JTextField("0");
        inputField.setAlignmentX(Component.CENTER_ALIGNMENT);
        panel.add(inputField);

        


        String[] choices = { "Час", "Відстань", "Швидкість", "Маса",
                            "Площа", "Температура", "Тиск", "Об’єм", "Енергія"};

        final JComboBox<String> cb = new JComboBox<String>(choices);

        cb.setMaximumSize(cb.getPreferredSize());
        cb.setAlignmentX(Component.CENTER_ALIGNMENT);
        panel.add(cb);

        


        JTextField outputField = new JTextField("0");
        outputField.setAlignmentX(Component.CENTER_ALIGNMENT);
        panel.add(outputField);


        JButton btn = new JButton("convert");
        btn.setAlignmentX(Component.CENTER_ALIGNMENT);
        panel.add(btn);


        btn.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                try{
                    float value  =Float.parseFloat(inputField.getText());

                    String x = String.valueOf(cb.getSelectedItem());

                    float result = convert(value,x);

                    outputField.setText(String.valueOf(result));
                } catch(NumberFormatException ex){
                    outputField.setText("Invalid input");
                }
            }
        });

        f.setVisible(true);
    }

    public static float convert (float num, String choice){
        switch(choice){
            case "Час":
                num /= 60; //sec to min
                break;
            
            case "Відстань":
                num /= 1000; //m to km
                break;

            case "Швидкість":
                num *= 3.6; // m\s to km\h
                break;

            case "Маса":
                num *= 1000; // kg to g
                break;

            case "Площа":
                num /= 1000000; //m^2 to km^2
                break;

            case "Температура":
                num = (num * (9/5)) + 32; // C to F
                break;

            case "Тиск":
                num /= 101325; // Pa to atm
                break;

            case "Об’єм":
                num *= 1000; // m^3 to L
                break;

            case "Енергія":
                num /= 3600000; // J to kW\h
                break;
        }
        return num;
    }
}
