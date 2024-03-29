
import java.awt.Point;
import java.awt.Color;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.awt.event.MouseMotionListener;
import javax.swing.JPanel;
import java.io.File;
import java.io.IOException;
import java.awt.image.*;
import javax.imageio.ImageIO;


public class PaintPanel extends JPanel
{
	private int pointCount = -1, undoCount = -1;
	private int toDraw, color;
	private int[] colorMatch = new int[10000], shape = new int[10000];
	private Point[] startPt = new Point[10000], endPt = new Point[10000];
	private Color[] colors = {Color.RED, Color.GREEN, Color.BLUE, Color.BLACK};
	private BufferedImage bi=null;//=new BufferedImage(500, 500, BufferedImage.TYPE_INT_ARGB);
	private Graphics2D g2d;//=bi.createGraphics();
	private int pic=0;
	
	public PaintPanel()
	{
		setBackground( Color.WHITE);
		MouseHandler handler = new MouseHandler();
		super.addMouseListener(handler);
		super.addMouseMotionListener(handler);
	}
	
	public void drawShape(int shape, int color)
	{
		this.toDraw = shape;
		this.color = color;
	}
	

	public void undo() 
	{
		if (pointCount == -1) {}
		else
		{
			pointCount--;
			undoCount++;
			repaint();
			
		}
	}
	
	public void redo() 
	{
		if (undoCount == -1) {}
		else
		{
			pointCount++;
			undoCount--;
			repaint();
		}
	}
	private class MouseHandler implements MouseListener, MouseMotionListener
	{
		public void mouseClicked(MouseEvent event)
		{
		}
		
		public void mousePressed(MouseEvent event)
		{
			pointCount++;
			startPt[pointCount] = event.getPoint();
			endPt[pointCount] = event.getPoint();
			shape[pointCount] = toDraw;
			colorMatch[pointCount] = color;
		}
		
		public void mouseReleased(MouseEvent event)
		{
			endPt[pointCount] = event.getPoint();
			repaint();
		}
		
		public void mouseEntered(MouseEvent event)
		{
		}
		
		public void mouseExited(MouseEvent event)
		{
		}
		
		public void mouseDragged(MouseEvent event)
		{
			endPt[pointCount] = event.getPoint();
			repaint();
		}
		
		public void mouseMoved(MouseEvent event)
		{
		}
	}


	public void paintComponent(Graphics g)
	{
		super.paintComponent(g);
		Graphics2D gc;
		g2d=(Graphics2D) g;
		if(bi==null) {
			bi=new BufferedImage(this.getWidth(), this.getHeight(), BufferedImage.TYPE_INT_ARGB);
			gc=bi.createGraphics();
			gc.setColor(Color.white);
			gc.fillRect(0,0,this.getWidth(),this.getHeight());
		}
		else {
			gc=bi.createGraphics();
		}
		
		//g.drawImage(bi, 0, 0, this);
		//g2d.setColor(Color.white);
		//g2d.fillRect(0,0,this.getWidth(),this.getHeight());
		for(int i = 0; i <= pointCount; i++)
		{
			//g.setColor(colors[colorMatch[i]]);
			gc.setColor(colors[colorMatch[i]]);
			if (shape[i] == 0) {
			/*g.drawRect(endPt[i].x - startPt[i].x > 0 ? startPt[i].x : endPt[i].x , 
					endPt[i].y - startPt[i].y > 0 ? startPt[i].y : endPt[i].y, 
					endPt[i].x - startPt[i].x > 0 ? endPt[i].x - startPt[i].x : startPt[i].x - endPt[i].x,
					endPt[i].y - startPt[i].y > 0 ? endPt[i].y - startPt[i].y : startPt[i].y - endPt[i].y);*/
			gc.drawRect(endPt[i].x - startPt[i].x > 0 ? startPt[i].x : endPt[i].x , 
					endPt[i].y - startPt[i].y > 0 ? startPt[i].y : endPt[i].y, 
					endPt[i].x - startPt[i].x > 0 ? endPt[i].x - startPt[i].x : startPt[i].x - endPt[i].x,
					endPt[i].y - startPt[i].y > 0 ? endPt[i].y - startPt[i].y : startPt[i].y - endPt[i].y);
			}
			else if (shape[i] == 1) {
			/*g.drawOval(endPt[i].x - startPt[i].x > 0 ? startPt[i].x : endPt[i].x , 
					endPt[i].y - startPt[i].y > 0 ? startPt[i].y : endPt[i].y, 
					endPt[i].x - startPt[i].x > 0 ? endPt[i].x - startPt[i].x : startPt[i].x - endPt[i].x,
					endPt[i].y - startPt[i].y > 0 ? endPt[i].y - startPt[i].y : startPt[i].y - endPt[i].y);*/
			gc.drawOval(endPt[i].x - startPt[i].x > 0 ? startPt[i].x : endPt[i].x , 
					endPt[i].y - startPt[i].y > 0 ? startPt[i].y : endPt[i].y, 
					endPt[i].x - startPt[i].x > 0 ? endPt[i].x - startPt[i].x : startPt[i].x - endPt[i].x,
					endPt[i].y - startPt[i].y > 0 ? endPt[i].y - startPt[i].y : startPt[i].y - endPt[i].y);
			}
			else if (shape[i] == 2) {
				/*g.drawLine(endPt[i].x, endPt[i].y, startPt[i].x, startPt[i].y);*/
				gc.drawLine(endPt[i].x, endPt[i].y, startPt[i].x, startPt[i].y);
			}
		}
		//g.drawImage(bi,0,0,this);
		gc.drawImage(bi, 0,0,this);
	}
	public void save() throws IOException
	{
		ImageIO.write(bi, "PNG", new File("filename.png"));
		System.out.println("Save complete.");
	}
	
	public void load() throws IOException
	{
		bi=ImageIO.read(new File("filename.png"));
		System.out.println("Reading complete.");
		g2d=bi.createGraphics();
		System.out.println(g2d);
		//g2d.drawImage(bi, 0, 0, this);
		pic=1;
	}
}
