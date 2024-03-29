import java.awt.*;
import javax.swing.*;
import java.awt.event.*;
import javax.swing.event.*;


public class MyFrame extends JFrame {
	private FigButtonPanel fbp=new FigButtonPanel();
	private ColorButtonPanel fbp2=new ColorButtonPanel();
	private PaintPanel pp=new PaintPanel();
	private JPanel board=new JPanel();
	public MyFrame() {
		add("North",fbp);
		board.setLayout(new BoxLayout(board, BoxLayout.Y_AXIS));
		pp.setLayout(new BorderLayout());
		//fbp2.setLayout(new BorderLayout());
		board.add("North",fbp2);
		board.add("Center",pp);
		add("Center",board);
		/*JPanel centerpanel=new JPanel();
		centerpanel.setLayout(new BorderLayout(10, 10));
		centerpanel.add("North",fbp);
		centerpanel.add(pp);
		add(centerpanel);*/
	}
}

class FigButtonPanel extends JPanel{
	private JButton[] btns=new JButton[5];
	public FigButtonPanel() {
		setLayout(new FlowLayout());
		btns[0]=new JButton("EmptyRect");
		btns[1]=new JButton("EmptyOval");
		btns[2]=new JButton("FilledRect");
		btns[3]=new JButton("FilledOval");
		btns[4]=new JButton("Line");
		//btns[0].setPreferredSize(new Dimension(100,30));
		add(btns[0]);
		//btns[1].setPreferredSize(new Dimension(100,30));
		add(btns[1]);
		add(btns[2]);
		add(btns[3]);
		add(btns[4]);
		setBackground(Color.white);
	}
	
}

class ColorButtonPanel extends JPanel{
	private JButton[] btns=new JButton[5];
	public ColorButtonPanel() {
		setLayout(new FlowLayout());
		btns[0]=new JButton("Black");
		btns[1]=new JButton("Red");
		btns[2]=new JButton("Blue");
		btns[3]=new JButton("Green");
		btns[4]=new JButton("Yellow");
		for(int i=0;i<5;i++) {
			add(btns[i]);
		}
		setBackground(Color.white);
	}
}
class PaintPanel extends JPanel{
	public PaintPanel() {
		setBackground(Color.red);
		//setSize(300,300);
	}
}