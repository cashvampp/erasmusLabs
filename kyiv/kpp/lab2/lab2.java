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
import javax.swing.ButtonGroup;
import javax.swing.JRadioButton;
import java.awt.Component;

class lab2 extends JFrame{
    public static void main(String[] args) {
        JFrame f = new JFrame("Конвертер величин");

        f.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        f.setSize(500, 500);
        f.setLocation(430, 100);

        JPanel panel = new JPanel();
        panel.setLayout(new BoxLayout(panel, BoxLayout.Y_AXIS));

        f.add(panel);

        JLabel lbl = new JLabel("Виберіть один із можливих варіантів та натисніть кнопку конвертації");
        lbl.setAlignmentX(Component.CENTER_ALIGNMENT);
        panel.add(lbl);

        // Radio buttons for conversion direction
        JPanel radioPanel = new JPanel();
        radioPanel.setAlignmentX(Component.CENTER_ALIGNMENT);
        ButtonGroup directionGroup = new ButtonGroup();
        
        JRadioButton sysToNonSys = new JRadioButton("Із системних в несистемні", true);
        JRadioButton nonSysToSys = new JRadioButton("Із несистемних в системні");
        
        directionGroup.add(sysToNonSys);
        directionGroup.add(nonSysToSys);
        
        radioPanel.add(sysToNonSys);
        radioPanel.add(nonSysToSys);
        panel.add(radioPanel);
        
        // Input field and label
        JLabel inputLabel = new JLabel("Введіть значення:");
        inputLabel.setAlignmentX(Component.CENTER_ALIGNMENT);
        panel.add(inputLabel);
        
        JTextField inputField = new JTextField("0");
        inputField.setAlignmentX(Component.CENTER_ALIGNMENT);
        inputField.setMaximumSize(inputField.getPreferredSize().width > 200 ? 
                              inputField.getPreferredSize() : 
                              new java.awt.Dimension(200, inputField.getPreferredSize().height));
        panel.add(inputField);
        
        // Dropdown for unit selection
        JLabel selectLabel = new JLabel("Виберіть тип величини:");
        selectLabel.setAlignmentX(Component.CENTER_ALIGNMENT);
        panel.add(selectLabel);

        String[] choices = { "Час", "Відстань", "Швидкість", "Маса",
                            "Площа", "Температура", "Тиск", "Об'єм", "Енергія"};

        final JComboBox<String> cb = new JComboBox<String>(choices);
        cb.setMaximumSize(cb.getPreferredSize());
        cb.setAlignmentX(Component.CENTER_ALIGNMENT);
        panel.add(cb);
        
        // Output field and label
        JLabel outputLabel = new JLabel("Результат:");
        outputLabel.setAlignmentX(Component.CENTER_ALIGNMENT);
        panel.add(outputLabel);

        JTextField outputField = new JTextField("0");
        outputField.setAlignmentX(Component.CENTER_ALIGNMENT);
        outputField.setMaximumSize(outputField.getPreferredSize().width > 200 ? 
                               outputField.getPreferredSize() : 
                               new java.awt.Dimension(200, outputField.getPreferredSize().height));
        panel.add(outputField);

        // Conversion button
        JButton btn = new JButton("Конвертувати");
        btn.setAlignmentX(Component.CENTER_ALIGNMENT);
        panel.add(btn);

        // Reset button
        JButton resetbtn = new JButton("Скинути");
        resetbtn.setAlignmentX(Component.CENTER_ALIGNMENT);
        panel.add(resetbtn);

        // Convert button functionality
        btn.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                try{
                    float value = Float.parseFloat(inputField.getText());
                    String unitType = String.valueOf(cb.getSelectedItem());
                    boolean isForward = sysToNonSys.isSelected();
                    
                    float result = convert(value, unitType, isForward);
                    outputField.setText(String.format("%.6f", result));
                } catch(NumberFormatException ex){
                    outputField.setText("Некоректне введення");
                }
            }
        });

        // Reset button functionality
        resetbtn.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                inputField.setText("0");
                outputField.setText("0");
                cb.setSelectedIndex(0);
                sysToNonSys.setSelected(true);
            }
        });

        f.setVisible(true);
    }

    // Enhanced conversion function with direction parameter
    public static float convert(float num, String choice, boolean isForward) {
        switch(choice) {
            case "Час":
                if (isForward)
                    num /= 60; // sec to min
                else
                    num *= 60; // min to sec
                break;
            
            case "Відстань":
                if (isForward)
                    num /= 1000; // m to km
                else
                    num *= 1000; // km to m
                break;

            case "Швидкість":
                if (isForward)
                    num *= 3.6; // m/s to km/h
                else
                    num /= 3.6; // km/h to m/s
                break;

            case "Маса":
                if (isForward)
                    num *= 1000; // kg to g
                else
                    num /= 1000; // g to kg
                break;

            case "Площа":
                if (isForward)
                    num /= 1000000; // m^2 to km^2
                else
                    num *= 1000000; // km^2 to m^2
                break;

            case "Температура":
                if (isForward)
                    num = (num * 9/5) + 32; // C to F
                else
                    num = (num - 32) * 5/9; // F to C
                break;

            case "Тиск":
                if (isForward)
                    num /= 101325; // Pa to atm
                else
                    num *= 101325; // atm to Pa
                break;

            case "Об'єм":
                if (isForward)
                    num *= 1000; // m^3 to L
                else
                    num /= 1000; // L to m^3
                break;

            case "Енергія":
                if (isForward)
                    num /= 3600000; // J to kWh
                else
                    num *= 3600000; // kWh to J
                break;
        }
        return num;
    }
}