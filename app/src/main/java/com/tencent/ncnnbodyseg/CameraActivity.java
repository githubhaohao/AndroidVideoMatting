/**
 *
 * Created by 公众号：字节流动 on 2021/3/16.
 * https://github.com/githubhaohao/LearnFFmpeg
 * 最新文章首发于公众号：字节流动，有疑问或者技术交流可以添加微信 Byte-Flow ,领取视频教程, 拉你进技术交流群
 *
 * */

package com.tencent.ncnnbodyseg;

import static com.tencent.ncnnbodyseg.media.CameraNativeContext.IMAGE_FORMAT_NV21;

import android.Manifest;
import android.annotation.SuppressLint;
import android.app.AlertDialog;
import android.content.Context;
import android.content.pm.PackageManager;
import android.graphics.Color;
import android.hardware.camera2.CameraCharacteristics;
import android.opengl.GLSurfaceView;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.util.DisplayMetrics;
import android.util.Log;
import android.util.Size;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewTreeObserver;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.annotation.RequiresApi;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import com.tencent.ncnnbodyseg.camera.Camera2FrameCallback;
import com.tencent.ncnnbodyseg.camera.Camera2Wrapper;
import com.tencent.ncnnbodyseg.camera.CameraUtil;
import com.tencent.ncnnbodyseg.media.MyNativeContext;

import java.io.File;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.GregorianCalendar;
import java.util.List;
import java.util.Locale;


public class CameraActivity extends AppCompatActivity implements Camera2FrameCallback, View.OnClickListener {
    private static final String TAG = "MainActivity";
    private static final SimpleDateFormat DateTime_FORMAT = new SimpleDateFormat("yyyyMMddHHmmss", Locale.US);
    private static final String RESULT_IMG_DIR = "byteflow/learnffmpeg";
    private static final String[] REQUEST_PERMISSIONS = {
            Manifest.permission.CAMERA,
            Manifest.permission.WRITE_EXTERNAL_STORAGE,
    };
    private static final int CAMERA_PERMISSION_REQUEST_CODE = 1;
    private RelativeLayout mSurfaceViewRoot;
    private Camera2Wrapper mCamera2Wrapper;
    private ImageButton mSwitchCamBtn, mSwitchRatioBtn;
    protected GLSurfaceView mGLSurfaceView;
    protected Size mRootViewSize, mScreenSize;
    private MyNativeContext mNativeContext = new MyNativeContext();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                WindowManager.LayoutParams.FLAG_FULLSCREEN);
        setContentView(R.layout.activity_camera);
        //Toolbar toolbar = findViewById(R.id.toolbar);
        //setSupportActionBar(toolbar);

//        FloatingActionButton fab = (FloatingActionButton) findViewById(R.id.fab);
//        fab.setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View view) {
//                if (mCamera2Wrapper != null) {
//                    mCamera2Wrapper.capture();
//                }
//            }
//        });
        mGLSurfaceView = new GLSurfaceView(this);

        initViews();
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (hasPermissionsGranted(REQUEST_PERMISSIONS)) {
            mCamera2Wrapper.startCamera();
        } else {
            ActivityCompat.requestPermissions(this, REQUEST_PERMISSIONS, CAMERA_PERMISSION_REQUEST_CODE);
        }
        updateTransformMatrix(mCamera2Wrapper.getCameraId());
        if (mSurfaceViewRoot != null) {
            updateGLSurfaceViewSize(mCamera2Wrapper.getPreviewSize());
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        if (requestCode == CAMERA_PERMISSION_REQUEST_CODE) {
            if (hasPermissionsGranted(REQUEST_PERMISSIONS)) {
                mCamera2Wrapper.startCamera();
            } else {
                Toast.makeText(this, "We need the camera permission.", Toast.LENGTH_SHORT).show();
            }
        } else {
            super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        }
    }

    @Override
    protected void onPause() {
        if (hasPermissionsGranted(REQUEST_PERMISSIONS)) {
            mCamera2Wrapper.stopCamera();
        }
        super.onPause();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mNativeContext.unInit();
    }

//    @Override
//    public boolean onCreateOptionsMenu(Menu menu) {
//        // Inflate the menu; this adds items to the action bar if it is present.
//        getMenuInflater().inflate(R.menu.menu_camera, menu);
//        return true;
//    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_change_resolution) {
            showChangeSizeDialog();
        } else if (id == R.id.action_switch_camera) {
            String cameraId = mCamera2Wrapper.getCameraId();
            String[] cameraIds = mCamera2Wrapper.getSupportCameraIds();
            if (cameraIds != null) {
                for (int i = 0; i < cameraIds.length; i++) {
                    if (!cameraIds[i].equals(cameraId)) {
                        mCamera2Wrapper.updateCameraId(cameraIds[i]);
                        updateTransformMatrix(cameraIds[i]);
                        updateGLSurfaceViewSize(mCamera2Wrapper.getPreviewSize());
                        break;
                    }
                }
            }
        }

        return true;
    }

    @Override
    public void onPreviewFrame(byte[] data, int width, int height) {
        Log.d(TAG, "onPreviewFrame() called with: data = [" + data + "], width = [" + width + "], height = [" + height + "]");
        mNativeContext.onPreviewFrame(IMAGE_FORMAT_NV21, data, width, height);
        mNativeContext.requestRender();
    }

    @Override
    public void onCaptureFrame(byte[] data, int width, int height) {
        Log.d(TAG, "onCaptureFrame() called with: data = [" + data + "], width = [" + width + "], height = [" + height + "]");
    }

    private void initViews() {
        mSwitchCamBtn = (ImageButton) findViewById(R.id.switch_camera_btn);
        //mSwitchRatioBtn = (ImageButton) findViewById(R.id.switch_ratio_btn);
        mSwitchCamBtn.bringToFront();
        //mSwitchRatioBtn.bringToFront();
        mSwitchCamBtn.setOnClickListener(this);
        //mSwitchRatioBtn.setOnClickListener(this);

        mSurfaceViewRoot = (RelativeLayout) findViewById(R.id.surface_root);
        RelativeLayout.LayoutParams p = new RelativeLayout.LayoutParams(RelativeLayout.LayoutParams.MATCH_PARENT,
                RelativeLayout.LayoutParams.MATCH_PARENT);
        mSurfaceViewRoot.addView(mGLSurfaceView, p);
        mNativeContext.init(this, getAssets(), mGLSurfaceView);

        mCamera2Wrapper = new Camera2Wrapper(this);
        //mCamera2Wrapper.setDefaultPreviewSize(getScreenSize());

        ViewTreeObserver treeObserver = mSurfaceViewRoot.getViewTreeObserver();
        treeObserver.addOnPreDrawListener(new ViewTreeObserver.OnPreDrawListener() {
            @Override
            public boolean  onPreDraw() {
                mSurfaceViewRoot.getViewTreeObserver().removeOnPreDrawListener(this);
                mRootViewSize = new Size(mSurfaceViewRoot.getMeasuredWidth(), mSurfaceViewRoot.getMeasuredHeight());
                updateGLSurfaceViewSize(mCamera2Wrapper.getPreviewSize());
                return true;
            }
        });
    }

    private void showChangeSizeDialog() {
        if (mCamera2Wrapper.getSupportPreviewSize() == null || mCamera2Wrapper.getSupportPreviewSize().size() == 0)
            return;
        if (mCamera2Wrapper.getSupportPictureSize() == null || mCamera2Wrapper.getSupportPictureSize().size() == 0)
            return;

        final ArrayList<String> previewSizeTitles = new ArrayList<>(mCamera2Wrapper.getSupportPreviewSize().size());
        int previewSizeSelectedIndex = 0;
        String selectedSize = mCamera2Wrapper.getPreviewSize().getWidth() + "x" + mCamera2Wrapper.getPreviewSize().getHeight();
        for (Size size : mCamera2Wrapper.getSupportPreviewSize()) {
            String title = size.getWidth() + "x" + size.getHeight();
            previewSizeTitles.add(title);
            if (selectedSize.equals(title)) {
                previewSizeSelectedIndex = previewSizeTitles.size() - 1;
            }
        }

        final ArrayList<String> captureSizeTitles = new ArrayList<>(mCamera2Wrapper.getSupportPreviewSize().size());
        int captureSizeSelectedIndex = 0;
        selectedSize = mCamera2Wrapper.getPictureSize().getWidth() + "x" + mCamera2Wrapper.getPictureSize().getHeight();
        for (Size size : mCamera2Wrapper.getSupportPictureSize()) {
            String title = size.getWidth() + "x" + size.getHeight();
            captureSizeTitles.add(title);
            if (selectedSize.equals(title)) {
                captureSizeSelectedIndex = captureSizeTitles.size() - 1;
            }
        }


        final AlertDialog.Builder builder = new AlertDialog.Builder(this);
        LayoutInflater inflater = LayoutInflater.from(this);
        final View rootView = inflater.inflate(R.layout.resolution_selected_layout, null);

        final AlertDialog dialog = builder.create();

        Button confirmBtn = (Button) rootView.findViewById(R.id.confirm_btn);
        confirmBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                dialog.cancel();
            }
        });

        final RecyclerView resolutionsListView = (RecyclerView) rootView.findViewById(R.id.resolution_list_view);

        final MyRecyclerViewAdapter myPreviewSizeViewAdapter = new MyRecyclerViewAdapter(this, previewSizeTitles);
        myPreviewSizeViewAdapter.setSelectIndex(previewSizeSelectedIndex);
        myPreviewSizeViewAdapter.addOnItemClickListener(new MyRecyclerViewAdapter.OnItemClickListener() {
            @Override
            public void onItemClick(View view, int position) {
                int selectIndex = myPreviewSizeViewAdapter.getSelectIndex();
                myPreviewSizeViewAdapter.setSelectIndex(position);
                myPreviewSizeViewAdapter.notifyItemChanged(selectIndex);
                myPreviewSizeViewAdapter.notifyItemChanged(position);

                String[] strs = previewSizeTitles.get(position).split("x");
                Size updateSize = new Size(Integer.valueOf(strs[0]), Integer.valueOf(strs[1]));
                Log.d(TAG, "onItemClick() called with: strs[0] = [" + strs[0] + "], strs[1] = [" + strs[1] + "]");
                mCamera2Wrapper.updatePreviewSize(updateSize);
                updateGLSurfaceViewSize(mCamera2Wrapper.getPreviewSize());
                dialog.cancel();
            }
        });

        final MyRecyclerViewAdapter myCaptureSizeViewAdapter = new MyRecyclerViewAdapter(this, previewSizeTitles);
        myCaptureSizeViewAdapter.setSelectIndex(captureSizeSelectedIndex);
        myCaptureSizeViewAdapter.addOnItemClickListener(new MyRecyclerViewAdapter.OnItemClickListener() {
            @Override
            public void onItemClick(View view, int position) {
                int selectIndex = myCaptureSizeViewAdapter.getSelectIndex();
                myCaptureSizeViewAdapter.setSelectIndex(position);
                myCaptureSizeViewAdapter.notifyItemChanged(selectIndex);
                myCaptureSizeViewAdapter.notifyItemChanged(position);

                String[] strs = captureSizeTitles.get(position).split("x");
                Size updateSize = new Size(Integer.valueOf(strs[0]), Integer.valueOf(strs[1]));
                Log.d(TAG, "onItemClick() called with: strs[0] = [" + strs[0] + "], strs[1] = [" + strs[1] + "]");
                mCamera2Wrapper.updatePictureSize(updateSize);
                updateGLSurfaceViewSize(mCamera2Wrapper.getPreviewSize());
                dialog.cancel();
            }
        });

        LinearLayoutManager manager = new LinearLayoutManager(this);
        manager.setOrientation(LinearLayoutManager.VERTICAL);
        resolutionsListView.setLayoutManager(manager);

        resolutionsListView.setAdapter(myPreviewSizeViewAdapter);
        resolutionsListView.scrollToPosition(previewSizeSelectedIndex);

        RadioGroup radioGroup = (RadioGroup) rootView.findViewById(R.id.radio_group);
        radioGroup.setOnCheckedChangeListener(new RadioGroup.OnCheckedChangeListener() {
            @RequiresApi(api = Build.VERSION_CODES.M)
            @SuppressLint("ResourceAsColor")
            @Override
            public void onCheckedChanged(RadioGroup group, int checkedId) {
                Log.d(TAG, "onCheckedChanged() called with: group = [" + group + "], checkedId = [" + checkedId + "]");
                if (checkedId == R.id.capture_size_btn) {
                    ((RadioButton) rootView.findViewById(R.id.capture_size_btn)).setTextColor(getColor(R.color.colorAccent));
                    ((RadioButton) rootView.findViewById(R.id.preview_size_btn)).setTextColor(getColor(R.color.colorText));
                    resolutionsListView.setAdapter(myCaptureSizeViewAdapter);
                    resolutionsListView.scrollToPosition(myCaptureSizeViewAdapter.getSelectIndex());

                } else {
                    ((RadioButton) rootView.findViewById(R.id.capture_size_btn)).setTextColor(getColor(R.color.colorText));
                    ((RadioButton) rootView.findViewById(R.id.preview_size_btn)).setTextColor(getColor(R.color.colorAccent));
                    resolutionsListView.setAdapter(myPreviewSizeViewAdapter);
                    resolutionsListView.scrollToPosition(myPreviewSizeViewAdapter.getSelectIndex());
                }
            }
        });

        dialog.show();
        dialog.getWindow().setContentView(rootView);

    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.switch_camera_btn:
                String cameraId = mCamera2Wrapper.getCameraId();
                String[] cameraIds = mCamera2Wrapper.getSupportCameraIds();
                if (cameraIds != null) {
                    for (int i = 0; i < cameraIds.length; i++) {
                        if (!cameraIds[i].equals(cameraId)) {
                            mCamera2Wrapper.updateCameraId(cameraIds[i]);
                            updateTransformMatrix(cameraIds[i]);
                            updateGLSurfaceViewSize(mCamera2Wrapper.getPreviewSize());
                            break;
                        }
                    }
                }
                break;
            case R.id.switch_ratio_btn:
                showChangeSizeDialog();
                break;
                default:
        }
    }

    protected boolean hasPermissionsGranted(String[] permissions) {
        for (String permission : permissions) {
            if (ActivityCompat.checkSelfPermission(this, permission)
                    != PackageManager.PERMISSION_GRANTED) {
                return false;
            }
        }
        return true;
    }

    public void updateTransformMatrix(String cameraId) {
        if (Integer.valueOf(cameraId) == CameraCharacteristics.LENS_FACING_FRONT) {
            mNativeContext.setTransformMatrix(90, 0);
        } else {
            mNativeContext.setTransformMatrix(90, 1);
        }

    }

    public void updateGLSurfaceViewSize(Size previewSize) {
        Size fitSize = null;
        fitSize = CameraUtil.getFitInScreenSize(previewSize.getWidth(), previewSize.getHeight(), getScreenSize().getWidth(), getScreenSize().getHeight());
        RelativeLayout.LayoutParams params = (RelativeLayout.LayoutParams) mGLSurfaceView
                .getLayoutParams();
        params.width = fitSize.getWidth();
        params.height = fitSize.getHeight();
        params.addRule(RelativeLayout.ALIGN_PARENT_TOP | RelativeLayout.CENTER_HORIZONTAL);

        mGLSurfaceView.setLayoutParams(params);
    }

    public Size getScreenSize() {
        if (mScreenSize == null) {
            DisplayMetrics displayMetrics = new DisplayMetrics();
            getWindowManager().getDefaultDisplay().getMetrics(displayMetrics);
            mScreenSize = new Size(displayMetrics.widthPixels, displayMetrics.heightPixels);
        }
        return mScreenSize;
    }

    public static final File getOutFile(final String ext) {
        final File dir = new File(Environment.getExternalStorageDirectory(), RESULT_IMG_DIR);
        Log.d(TAG, "path=" + dir.toString());
        dir.mkdirs();
        if (dir.canWrite()) {
            return new File(dir, "video_" + getDateTimeString() + ext);
        }
        return null;
    }

    private static final String getDateTimeString() {
        final GregorianCalendar now = new GregorianCalendar();
        return DateTime_FORMAT.format(now.getTime());
    }

    public static class MyRecyclerViewAdapter extends RecyclerView.Adapter<MyRecyclerViewAdapter.MyViewHolder> implements View.OnClickListener {
        private List<String> mTitles;
        private Context mContext;
        private int mSelectIndex = 0;
        private OnItemClickListener mOnItemClickListener = null;

        public MyRecyclerViewAdapter(Context context, List<String> titles) {
            mContext = context;
            mTitles = titles;
        }

        public void setSelectIndex(int index) {
            mSelectIndex = index;
        }

        public int getSelectIndex() {
            return mSelectIndex;
        }

        public void addOnItemClickListener(OnItemClickListener onItemClickListener) {
            mOnItemClickListener = onItemClickListener;
        }

        @NonNull
        @Override
        public MyViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
            View view = LayoutInflater.from(parent.getContext()).inflate(R.layout.resolution_item_layout, parent, false);
            MyViewHolder myViewHolder = new MyViewHolder(view);
            view.setOnClickListener(this);
            return myViewHolder;
        }

        @Override
        public void onBindViewHolder(@NonNull MyViewHolder holder, int position) {
            holder.mTitle.setText(mTitles.get(position));
            if (position == mSelectIndex) {
                holder.mRadioButton.setChecked(true);
                holder.mTitle.setTextColor(mContext.getResources().getColor(R.color.colorAccent));
            } else {
                holder.mRadioButton.setChecked(false);
                holder.mTitle.setText(mTitles.get(position));
                holder.mTitle.setTextColor(Color.GRAY);
            }
            holder.itemView.setTag(position);
        }

        @Override
        public int getItemCount() {
            return mTitles.size();
        }

        @Override
        public void onClick(View v) {
            if (mOnItemClickListener != null) {
                mOnItemClickListener.onItemClick(v, (Integer) v.getTag());
            }
        }

        public interface OnItemClickListener {
            void onItemClick(View view, int position);
        }

        class MyViewHolder extends RecyclerView.ViewHolder {
            RadioButton mRadioButton;
            TextView mTitle;

            public MyViewHolder(View itemView) {
                super(itemView);
                mRadioButton = (RadioButton) itemView.findViewById(R.id.radio_btn);
                mTitle = (TextView) itemView.findViewById(R.id.item_title);
            }
        }
    }
}
