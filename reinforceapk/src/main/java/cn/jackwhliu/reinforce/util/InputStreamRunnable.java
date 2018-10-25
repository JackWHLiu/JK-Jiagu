package cn.jackwhliu.reinforce.util;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.PrintStream;

public class InputStreamRunnable extends Thread {

    private BufferedReader mInStream;
    private PrintStream mOutStreadm;

    /* package */ InputStreamRunnable(InputStream in, PrintStream out) {
        mInStream = new BufferedReader(new InputStreamReader(in));
        mOutStreadm = out;
    }

    @Override
    public void run() {
        String line;
        try {
            while ((line = mInStream.readLine()) != null) {
                mOutStreadm.println(line);
                Thread.sleep(1);
            }
            mInStream.close();
        } catch (IOException e) {
            e.printStackTrace();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }
}
