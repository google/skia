package org.skia.jetskidemo;

import android.content.Context;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Spinner;

import java.util.ArrayList;

public class NavigationSpinner extends Spinner {
    private final String TAG = "ANDROIDKIT DEMO SPINNER";

    public NavigationSpinner(Context context) {
        super(context);
        init(context);
    }
    public NavigationSpinner(Context context, AttributeSet attr) {
        super(context, attr);
        init(context);
    }

    private void init(Context context) {
        ArrayAdapter<String> adapter;

        // populate Spinner
        ArrayList<String> navigationOptions;
        try {
            navigationOptions = getActivityList(context);
        } catch (Exception e) {
            return;
        }
        adapter = new ArrayAdapter(context, android.R.layout.simple_spinner_item, navigationOptions);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        this.setAdapter(adapter);

        // set Spinner logic
        this.setOnItemSelectedListener(new OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                if (!parent.getItemAtPosition(position).equals("Select Activity")) {
                    String activityName = parent.getItemAtPosition(position).toString();
                    Log.d(TAG, "Navigating to " + activityName);
                    try {
                        Intent myIntent = new Intent(context, Class.forName(activityName));
                        context.startActivity(myIntent);
                    } catch (Exception e) {
                        Log.d(TAG, "Couldn't find selected activity.");
                    }
                }
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {

            }
        });
    }

    private ArrayList<String> getActivityList(Context c) throws PackageManager.NameNotFoundException {
        PackageManager pm = c.getPackageManager();
        PackageInfo info = pm.getPackageInfo(c.getPackageName(), PackageManager.GET_ACTIVITIES);
        ActivityInfo[] list = info.activities;

        ArrayList<String> activityNames = new ArrayList<>();
        for (ActivityInfo activity : list) {
            activityNames.add(activity.name);
        }
        activityNames.add(0, "Select Demo");
        return activityNames;
    }
}
